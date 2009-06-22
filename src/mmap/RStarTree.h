
/*
 *	This is intended to be a templated implementation of an R* Tree, designed
 *	to create an efficient and (relatively) small indexing container in N 
 *	dimensions. At the moment, it is a memory-based container instead of disk
 *  based.
 *
 *  inspired by Dustin Spicuzza <dustin@virtualroadside.com>
 *	Based on "The R*-Tree: An Efficient and Robust Access Method for Points 
 *	and Rectangles" by N. Beckmann, H.P. Kriegel, R. Schneider, and B. Seeger
 */

#ifndef RSTARTREE_H
#define RSTARTREE_H

#include "G3D/Array.h"
#include "G3D/AABox.h"
//#include "G3D/Sphere.h"


// R* tree parameters
#define RTREE_REINSERT_P 0.30
#define RTREE_CHOOSE_SUBTREE_P 32

using namespace G3D;

//void create(void* n) { assert(false); };

template<class T>
struct RStarLeaf : AABox {
	
	//typedef T leaf_type;
	T leaf;
};

struct RSTarNode : AABox {
    std::vector< AABox* > items;
    //AABox bound;
    bool hasLeaves;
};

/**********************************************************
 * R* Tree related functors used for sorting BoundedItems
 *
 * TODO: Take advantage of type traits
 **********************************************************/


/**
	\class RStarTree
	\brief Implementation of an RTree with an R* index
	
	@tparam T        		type of leaves stored in the tree
	@tparam	min_child_items m, in the range 2 <= m < M
	@tparam max_child_items M, in the range 2 <= m < M
*/
template<class T> class RStarTree {
private:
    std::size_t min_child_items;
    std::size_t max_child_items;
    RSTarNode* m_root;
    std::size_t m_size;

    
public:
    AABox BoundingBox;
    typedef RStarLeaf<T> Leaf;
    
    RStarTree (std::size_t p_min_child_items, std::size_t p_max_child_items) : min_child_items(p_min_child_items), max_child_items(p_max_child_items) ,m_root(NULL), m_size(0) 
        {
        assert(1 <= min_child_items && min_child_items <= max_child_items/2);
        }
        
	// Single insert function, adds a new item to the tree
	void Insert(T leaf)
	{
		// ID1: Invoke Insert starting with the leaf level as a
		// parameter, to Insert a new data rectangle
		
		Leaf * newLeaf = new Leaf();
		newLeaf->set(leaf->getBounds().low() , leaf->getBounds().high());
		newLeaf->leaf  = leaf;
		
		// create a new root node if necessary
		if (!m_root)
		{
			m_root = new RSTarNode();
			m_root->hasLeaves = true;
			
			// reserve memory
			m_root->items.reserve(min_child_items);
			m_root->items.push_back(newLeaf);
			m_root->set(leaf->getBounds().low() , leaf->getBounds().high());
		}
		else
			// start the insertion process
			InsertInternal(newLeaf, m_root);
			
		m_size += 1;
//	printf("m_root after insert : %f,%f %f,%f\n",m_root->low().x,m_root->low().y,m_root->high().x,m_root->high().y);
	}
	
	std::size_t GetSize() const { return m_size; }

	// choose subtree: only pass this items that do not have leaves
	// I took out the loop portion of this algorithm, so it only
	// picks a subtree at that particular level
	RSTarNode * ChooseSubtree(RSTarNode * node, const AABox * bound)
	{
		// If the child pointers in N point to leaves 
		if (static_cast<RSTarNode*>(node->items[0])->hasLeaves)
		{
			// determine the minimum overlap cost
			if (max_child_items > (RTREE_CHOOSE_SUBTREE_P*2)/3  && node->items.size() > RTREE_CHOOSE_SUBTREE_P)
			{
				// ** alternative algorithm:
				// Sort the rectangles in N in increasing order of
				// then area enlargement needed to include the new
				// data rectangle
				
				// Let A be the group of the first p entrles
				std::partial_sort( node->items.begin(), node->items.begin() + RTREE_CHOOSE_SUBTREE_P, node->items.end(), 
					SortNodesByAreaEnlargement(bound));
				
				// From the items in A, considering all items in
				// N, choose the leaf whose rectangle needs least
				// overlap enlargement
				
				return static_cast<RSTarNode*>(* std::min_element(node->items.begin(), node->items.begin() + RTREE_CHOOSE_SUBTREE_P,
					SortNodesByOverlapEnlargement(bound)));
			}

			// choose the leaf in N whose rectangle needs least
			// overlap enlargement to include the new data
			// rectangle Resolve ties by choosmg the leaf
			// whose rectangle needs least area enlargement, then
			// the leaf with the rectangle of smallest area
			
			return static_cast<RSTarNode*>(* std::min_element(node->items.begin(), node->items.end(),
				SortNodesByOverlapEnlargement(bound)));
		}
		
		// if the chlld pointers in N do not point to leaves

		// [determine the minimum area cost],
		// choose the leaf in N whose rectangle needs least
		// area enlargement to include the new data
		// rectangle. Resolve ties by choosing the leaf
		// with the rectangle of smallest area
			
		return static_cast<RSTarNode*>(* std::min_element( node->items.begin(), node->items.end(),
				SortNodesByAreaEnlargement(bound)));
	}
	
	
	// inserts nodes recursively. As an optimization, the algorithm steps are
	// way out of order. :) If this returns something, then that item should
	// be added to the caller's level of the tree
	RSTarNode * InsertInternal(Leaf* leaf, RSTarNode * node, bool firstInsert = true)
	{
		// I4: Adjust all covering rectangles in the insertion path
		// such that they are minimum bounding boxes
		// enclosing the children rectangles
		AABox lbounds;
		leaf->getBounds(lbounds);
	//	printf("InsertInternal leaf :  %f,%f %f,%f %s\n",lbounds.low().x,lbounds.low().y,lbounds.high().x,lbounds.high().y,(firstInsert?"first":""));
	//	printf("InsertInternal node :  %f,%f %f,%f %s\n",node->low().x,node->low().y,node->high().x,node->high().y,(node==m_root?"root":""));

		/*AABox nbounds=node->intersect(lbounds);
		node->set(nbounds.low(), nbounds.high());*/
		
		node->merge(lbounds);
	//	printf("stretched      node :  %f,%f %f,%f %s\n",node->low().x,node->low().y,node->high().x,node->high().y,(node==m_root?"root":""));

		//debug :
		//assert (firstInsert || node!=m_root);
	
		// CS2: If we're at a leaf, then use that level
		if (node->hasLeaves)
		{
			// I2: If N has less than M items, accommodate E in N
			node->items.push_back(leaf);
		}
		else
		{
			// I1: Invoke ChooseSubtree. with the level as a parameter,
			// to find an appropriate node N, m which to place the
			// new leaf E
		
			// of course, this already does all of that recursively. we just need to
			// determine whether we need to split the overflow or not
			RSTarNode * tmp_node = InsertInternal( leaf, ChooseSubtree(node, leaf), firstInsert );
			
			if (!tmp_node)
				return NULL;
				
			// this gets joined to the list of items at this level
			node->items.push_back(tmp_node);
		}
		
		
		// If N has M+1 items. invoke OverflowTreatment with the
		// level of N as a parameter [for reinsertion or split]
		if (node->items.size() > max_child_items )
		{
			
			// I3: If OverflowTreatment was called and a split was
			// performed, propagate OverflowTreatment upwards
			// if necessary
			
			// This is implicit, the rest of the algorithm takes place in there
			return OverflowTreatment(node, firstInsert);
		}
			
		return NULL;
	}
	

	// TODO: probably could just merge this in with InsertInternal()
	RSTarNode * OverflowTreatment(RSTarNode * level, bool firstInsert)
	{
		// OT1: If the level is not the root level AND this is the first
		// call of OverflowTreatment in the given level during the 
		// insertion of one data rectangle, then invoke Reinsert
		if (level != m_root && firstInsert)
		{
			Reinsert(level);
			return NULL;
		}
		
		RSTarNode * splitItem = Split(level);
		
		// If OverflowTreatment caused a split of the root, create a new root
		if (level == m_root)
		{
//printf("#########New ROOT\n");
			RSTarNode * newRoot = new RSTarNode();
			newRoot->hasLeaves = false;
			
			// reserve memory
			newRoot->items.reserve(min_child_items);
			newRoot->items.push_back(m_root);
			newRoot->items.push_back(splitItem);
			
			// Do I4 here for the new root item
			newRoot->set(Vector3::maxFinite(),Vector3::minFinite());
			for_each(newRoot->items.begin(), newRoot->items.end(), StretchAABox(newRoot));
			
			// and we're done
			m_root = newRoot;
			return NULL;
		}

		// propagate it upwards
		return splitItem;
	}
	
	// this combines Split, ChooseSplitAxis, and ChooseSplitIndex into 
	// one function as an optimization (they all share data structures,
	// so it would be pointless to do all of that copying)
	//
	// This returns a node, which should be added to the items of the
	// passed node's parent
	RSTarNode * Split(RSTarNode * node)
	{
		RSTarNode * newNode = new RSTarNode();
		newNode->hasLeaves = node->hasLeaves;

		const std::size_t n_items = node->items.size();
		const std::size_t distribution_count = n_items - 2*min_child_items + 1;
		
		std::size_t split_axis = 4 /*dimensions+1*/, split_edge = 0, split_index = 0;
		int split_margin = 0;
		
		AABox R1, R2;

		// these should always hold true
		assert(n_items == max_child_items + 1);
		assert(distribution_count > 0);
		assert(min_child_items + distribution_count-1 <= n_items);
		
		// S1: Invoke ChooseSplitAxis to determine the axis,
		// perpendicular to which the split 1s performed
		// S2: Invoke ChooseSplitIndex to determine the best
		// distribution into two groups along that axis
		
		// NOTE: We don't compare against node->bound, so it gets overwritten
		// at the end of the loop
		
		// CSA1: For each axis
		for (std::size_t axis = 0; axis < 3 /*dimensions*/; axis++)
		{
			// initialize per-loop items
			int margin = 0;
			double overlap = 0, dist_area, dist_overlap;
			std::size_t dist_edge = 0, dist_index = 0;
		
			dist_area = dist_overlap = std::numeric_limits<double>::max();
			
			
			// Sort the items by the lower then by the upper
			// edge of their bounding box on this particular axis and 
			// determine all distributions as described . Compute S. the
			// sum of all margin-values of the different
			// distributions
		
			// lower edge == 0, upper edge = 1
			for (std::size_t edge = 0; edge < 2; edge++)
			{
				// sort the items by the correct key (upper edge, lower edge)
				if (edge == 0)
					std::sort(node->items.begin(), node->items.end(), SortNodesByFirstEdge(axis));
				else
					std::sort(node->items.begin(), node->items.end(), SortNodesBySecondEdge(axis));
		
				// Distributions: pick a point m in the middle of the thing, call the left
				// R1 and the right R2. Calculate the bounding box of R1 and R2, then 
				// calculate the margins. Then do it again for some more points	
				for (std::size_t k = 0; k < distribution_count; k++)
		        {
					double area = 0;
				
					// calculate bounding box of R1
					R1.set(Vector3::maxFinite(),Vector3::minFinite());
					for_each(node->items.begin(), node->items.begin()+(min_child_items+k), StretchAABox(&R1));
							
					// then do the same for R2
					R2.set(Vector3::maxFinite(),Vector3::minFinite());
					for_each(node->items.begin()+(min_child_items+k+1), node->items.end(), StretchAABox(&R2));
					
					
					// calculate the three values
					margin 	+= R1.high().x-R1.low().x + 
					           R1.high().y-R1.low().y +
					           R1.high().z-R1.low().z +
					           R2.high().x-R2.low().x + 
 					           R2.high().y-R2.low().y +
 					           R2.high().z-R2.low().z ;
					           //R1.edgeDeltas() + R2.edgeDeltas();
					area 	+= R1.area() + R2.area();		// TODO: need to subtract.. overlap?
					overlap =  (R1.intersect(R2)).area();
					
					
					// CSI1: Along the split axis, choose the distribution with the 
					// minimum overlap-value. Resolve ties by choosing the distribution
					// with minimum area-value. 
					if (overlap < dist_overlap || (overlap == dist_overlap && area < dist_area))
					{
						// if so, store the parameters that allow us to recreate it at the end
						dist_edge = 	edge;
						dist_index = 	min_child_items+k;
						dist_overlap = 	overlap;
						dist_area = 	area;
					}		
				}
			}
			
			// CSA2: Choose the axis with the minimum S as split axis
			if (split_axis == 4 /*dimensions+1*/ || split_margin > margin )
			{
				split_axis 		= axis;
				split_margin 	= margin;
				split_edge 		= dist_edge;
				split_index 	= dist_index;
			}
		}
	
		// S3: Distribute the items into two groups
	
		// ok, we're done, and the best distribution on the selected split
		// axis has been recorded, so we just have to recreate it and
		// return the correct index
		
		if (split_edge == 0)
			std::sort(node->items.begin(), node->items.end(), SortNodesByFirstEdge(split_axis));

		// only reinsert the sort key if we have to
		else if (split_axis != 2/*dimensions-1*/)
			std::sort(node->items.begin(), node->items.end(), SortNodesBySecondEdge(split_axis));	
		
		// distribute the end of the array to the new node, then erase them from the original node
		newNode->items.assign(node->items.begin() + split_index, node->items.end());
		node->items.erase(node->items.begin() + split_index, node->items.end());
		
		// adjust the bounding box for each 'new' node
		node->set(Vector3::maxFinite(),Vector3::minFinite());
		std::for_each(node->items.begin(), node->items.end(), StretchAABox(node));
		
		newNode->set(Vector3::maxFinite(),Vector3::minFinite());
		std::for_each(newNode->items.begin(), newNode->items.end(), StretchAABox(newNode));
		
		return newNode;
	}
	
	// This routine is used to do the opportunistic reinsertion that the
	// R* algorithm calls for
	void Reinsert(RSTarNode * node)
	{
		std::vector< /*RSTarNode*/ AABox* > removed_items;

		const std::size_t n_items = node->items.size();
		const std::size_t p = (std::size_t)((double)n_items * RTREE_REINSERT_P) > 0 ? (std::size_t)((double)n_items * RTREE_REINSERT_P) : 1;
		
		// RI1 For all M+l items of a node N, compute the distance
		// between the centers of their rectangles and the center
		// of the bounding rectangle of N
		assert(n_items == max_child_items + 1);
		
		// RI2: Sort the items in increasing order of their distances
		// computed in RI1
		std::partial_sort(node->items.begin(), node->items.end() - p, node->items.end(), 
			SortNodesByDistanceFromCenter(node));
			
		// RI3.A: Remove the last p items from N
		removed_items.assign(node->items.end() - p, node->items.end());
		node->items.erase(node->items.end() - p, node->items.end());
		
		// RI3.B: adjust the bounding rectangle of N
		node->set(Vector3::maxFinite(),Vector3::minFinite());
		for_each(node->items.begin(), node->items.end(), StretchAABox(node));
		
		// RI4: In the sort, defined in RI2, starting with the 
		// minimum distance (= close reinsert), invoke Insert 
		// to reinsert the items
		for (typename std::vector< /*RSTarNode*/ AABox* >::iterator it = removed_items.begin(); it != removed_items.end(); it++)
			InsertInternal( (Leaf*)*it, m_root, false);
	}
	
	/****************************************************************
	 * These are used to implement walking the entire R* tree in a
	 * conditional way
	 ****************************************************************/
	
	
	bool
	_FindOneLeafContaining(const Vector3* pt,RSTarNode* node,T* leaf)
	{
		if (node->hasLeaves)
		{
//			printf("hasLeaves :\n");
			for( std::vector< /*Leaf*/ AABox* >::iterator it = node->items.begin(); it < node->items.end(); ++it)
			{
//			printf("? %f,%f %f,%f %f,%f\n",(*it)->low().x,(*it)->low().y,(*it)->high().x,(*it)->high().y,(*it)->low().z,(*it)->high().z);
				if ((*it)->contains(*pt))
					{
					
					*leaf = ((Leaf*)(*it))->leaf;
//					printf("FOUND MZ %d\n",(*leaf)->getIndex());
					return true;
					}
			}
		}
		else
		{
//			printf("hasNodes :\n");
			for(typename std::vector< /*RSTarNode*/ AABox* >::iterator it = node->items.begin(); it < node->items.end(); ++it)
			{
//			printf("? %f,%f %f,%f %f,%f\n",(*it)->low().x,(*it)->low().y,(*it)->high().x,(*it)->high().y,(*it)->low().z,(*it)->high().z);
				if ((*it)->contains(*pt))
				{
//					printf(">");
					if (_FindOneLeafContaining(pt,(RSTarNode*)(*it),leaf))
						{
//						printf("returning MZ %d\n",(*leaf)->getIndex());
						return true;
						
						}
				}
			}
		}
		return false;
	}
	
	T
	FindOneLeafContaining(const Vector3* pt)
	{
		T leaf = NULL;
//		printf("FindOneLeafContaining %f,%f,%f :\n",pt->x,pt->y,pt->z);
		if (m_root)
			_FindOneLeafContaining(pt,m_root,&leaf);
		return leaf;
	}

	bool
	_FindLeavesByZRange(const float x, const float y, const float lowZ, const float highZ,RSTarNode* node,Array<T>* leaves)
	{
		if (node->hasLeaves)
		{
//			printf("hasLeaves :\n");
			for( std::vector<AABox* >::iterator it = node->items.begin(); it < node->items.end(); ++it)
			{
//			printf("? %f,%f %f,%f %f,%f\n",(*it)->low().x,(*it)->low().y,(*it)->high().x,(*it)->high().y,(*it)->low().z,(*it)->high().z);
				if ( x > (*it)->low().x && x < (*it)->high().x &&
				     y > (*it)->low().y && y < (*it)->high().y &&
				     lowZ < (*it)->high().z &&
				     highZ > (*it)->low().z )
					{
					
					leaves->append(((Leaf*)(*it))->leaf);
//					printf("FOUND MZ %d\n",((Leaf*)(*it))->leaf->getIndex());
					//return true;
					}
			}
		}
		else
		{
//			printf("hasNodes :\n");
			for(typename std::vector< /*RSTarNode*/ AABox* >::iterator it = node->items.begin(); it < node->items.end(); ++it)
			{
//			printf("? %f,%f %f,%f %f,%f\n",(*it)->low().x,(*it)->low().y,(*it)->high().x,(*it)->high().y,(*it)->low().z,(*it)->high().z);
				if ( x > (*it)->low().x && x < (*it)->high().x &&
				     y > (*it)->low().y && y < (*it)->high().y &&
				     lowZ < (*it)->high().z &&
				     highZ > (*it)->low().z )
				{
//				    printf(">");
				    _FindLeavesByZRange(x, y, lowZ, highZ ,(RSTarNode*)(*it),leaves);
					
				    /*
					if (_FindFindLeafsByZRange(x, y, lowZ, highZ ,(RSTarNode*)(*it),leaves))
						{
//						printf("returning MZ %d\n",(*leaf)->getIndex());
						return true;
						
						}*/
				}
			}
		}
		return false;
	}
	
	Array<T>
	FindLeavesByZRange(const float x, const float y, const float lowZ, const float highZ)
	{
		Array<T> leaves = NULL;
		if (m_root)
			_FindLeavesByZRange(x, y, lowZ, highZ ,m_root,&leaves);
		return leaves;
	}

	/****************************************************************
	 * Used to get the tree structure as an array.
	 * should be used to debug / explore
	 ****************************************************************/
	bool
	_getLeavesArray(RSTarNode* node,Array<T>* leaves)
	{
		if (node->hasLeaves)
		{
			for( std::vector<AABox* >::iterator it = node->items.begin(); it < node->items.end(); ++it)
			{
				leaves->append(((Leaf*)(*it))->leaf);
			}
		}
		else
		{
			for(typename std::vector< AABox* >::iterator it = node->items.begin(); it < node->items.end(); ++it)
			{
				_getLeavesArray((RSTarNode*)(*it),leaves);
			}
		}
		return false;
	}
	
	/*Array<T>
	getLeavesArray()
	{
		Array<T> leaves = NULL;
		if (m_root)
			_getLeavesArray(m_root,&leaves);
		return leaves;
	}*/



	/****************************************************************
	 * Used to save the tree structure and leaves to a file.
	 *
	 * leaf save(FILE* fp) must be implemented
	 ****************************************************************/
    void
    _save(FILE* fp,RSTarNode* node)
    {
    	fwrite (&node->hasLeaves,1,1,fp);
    	size_t nbItem = node->items.size(); 
		fwrite (&nbItem,sizeof(size_t),1,fp);
		if (node->hasLeaves)
		{
			for( std::vector<AABox* >::iterator it = node->items.begin(); it < node->items.end(); ++it)
			{
			
				fwrite (&(*it)->low().x,sizeof(float),1,fp);
				fwrite (&(*it)->low().y,sizeof(float),1,fp);
				fwrite (&(*it)->low().z,sizeof(float),1,fp);
				fwrite (&(*it)->high().x,sizeof(float),1,fp);
				fwrite (&(*it)->high().y,sizeof(float),1,fp);
				fwrite (&(*it)->high().z,sizeof(float),1,fp);
				((Leaf*)(*it))->leaf->save(fp);
			}
		}
		else
		{
			for(typename std::vector< /*RSTarNode*/ AABox* >::iterator it = node->items.begin(); it < node->items.end(); ++it)
			{
				fwrite (&(*it)->low().x,sizeof(float),1,fp);
				fwrite (&(*it)->low().y,sizeof(float),1,fp);
				fwrite (&(*it)->low().z,sizeof(float),1,fp);
				fwrite (&(*it)->high().x,sizeof(float),1,fp);
				fwrite (&(*it)->high().y,sizeof(float),1,fp);
				fwrite (&(*it)->high().z,sizeof(float),1,fp);
				_save(fp ,(RSTarNode*)(*it));
			}
		}
    }

    void
    save(FILE* fp)
    {
    if (m_root)
	  _save(fp ,m_root);
    }

	/****************************************************************
	 * Used to load the tree structure and leaves from a file.
	 *
	 * leaf load(FILE* fp) must be implemented
	 * inline void create(T &alloc); must be overloaded, set alloc to an empty allocated T
	 ****************************************************************/
	 	 
    void
    _load(FILE* fp,RSTarNode* node,Array<T>* leaves)
    {
    	fread (&node->hasLeaves,1,1,fp);
    	//printf("hasLeaves : %s\n",(node->hasLeaves?"y":"n"));

    	size_t nbItem;
		fread (&nbItem,sizeof(size_t),1,fp);
    	//printf("%u childs\n",nbItem);
		if (node->hasLeaves)
		{
		//printf("L");

			for( unsigned int i = 0; i<nbItem; ++i)
			{
				//printf(".");

				Leaf * newLeaf = new Leaf();
			
				float x1,y1,z1,x2,y2,z2;
				fread (&x1,sizeof(float),1,fp);
				fread (&y1,sizeof(float),1,fp);
				fread (&z1,sizeof(float),1,fp);
				fread (&x2,sizeof(float),1,fp);
				fread (&y2,sizeof(float),1,fp);
				fread (&z2,sizeof(float),1,fp);
				
				//newLeaf->leaf = new <class T>();
				newLeaf->set(Vector3(x1,y1,z1),Vector3(x2,y2,z2));
				constructor(newLeaf->leaf);
				newLeaf->leaf->load(fp);
				if (leaves != NULL)
				  {
				  leaves->push_back(newLeaf->leaf);
				  }
				node->items.push_back(newLeaf);
				m_size += 1;
			}
		}
		else
		{
			//printf("N");

			for( unsigned int i = 0; i<nbItem; ++i)
			{
				//printf(".");

				RSTarNode * newNode = new RSTarNode();

				float x1,y1,z1,x2,y2,z2;
				fread (&x1,sizeof(float),1,fp);
				fread (&y1,sizeof(float),1,fp);
				fread (&z1,sizeof(float),1,fp);
				fread (&x2,sizeof(float),1,fp);
				fread (&y2,sizeof(float),1,fp);
				fread (&z2,sizeof(float),1,fp);
				newNode->set(Vector3(x1,y1,z1),Vector3(x2,y2,z2));
				node->items.push_back(newNode);
				_load(fp ,newNode,leaves);
			}
		}
    }

    /*void
    load(FILE* fp)
    {
    printf("RSTarNode\n");
	m_root = new RSTarNode();
    printf("reserve\n");
	m_root->items.reserve(min_child_items);
    printf("_load\n");

	_load(fp ,m_root,NULL);
    }*/

    Array<T>
    load(FILE* fp)
    {
    Array<T> leaves = NULL;
    //printf("RSTarNode\n");
	m_root = new RSTarNode();
    //printf("reserve\n");
	m_root->items.reserve(min_child_items);
    //printf("_load\n");

	_load(fp ,m_root,&leaves);
	return leaves;
    }
/*
	// visits a node if necessary
	template <typename Acceptor, typename Visitor>
	struct VisitFunctor : std::unary_function< const BoundingBox *, void > {
	
		const Acceptor &accept;
		Visitor &visit;
		
		explicit VisitFunctor(const Acceptor &a, Visitor &v) : accept(a), visit(v) {}
	
		void operator()( BoundedItem * item ) 
		{
			Leaf * leaf = static_cast<Leaf*>(item);
		
			if (accept(leaf))
				visit(leaf);
		}
	};
	
	
	// this functor recursively walks the tree
	template <typename Acceptor, typename Visitor>
	struct QueryFunctor : std::unary_function< const BoundedItem, void > {
		const Acceptor &accept;
		Visitor &visitor;
		
		explicit QueryFunctor(const Acceptor &a, Visitor &v) : accept(a), visitor(v) {}
	
		void operator()(BoundedItem * item)
		{
			Node * node = static_cast<Node*>(item);
		
			if (visitor.ContinueVisiting && accept(node))
			{
				if (node->hasLeaves)
					for_each(node->items.begin(), node->items.end(), VisitFunctor<Acceptor, Visitor>(accept, visitor));
				else
					for_each(node->items.begin(), node->items.end(), *this);
			}
		}
	};
	*/
	
	/****************************************************************
	 * Used to remove items from the tree
	 *
	 * At some point, the complexity just gets ridiculous. I'm pretty
	 * sure that the remove functions are close to that by now... 
	 ****************************************************************/
	
/*
	
	// determines whether a leaf should be deleted or not
	template <typename Acceptor, typename LeafRemover>
	struct RemoveLeafFunctor : 
		std::unary_function< const BoundingBox *, bool > 
	{
		const Acceptor &accept;
		LeafRemover &remove;
		std::size_t * size;
		
		explicit RemoveLeafFunctor(const Acceptor &a, LeafRemover &r, std::size_t * s) :
			accept(a), remove(r), size(s) {}
	
		bool operator()(BoundedItem * item ) const {
			Leaf * leaf = static_cast<Leaf *>(item);
			
			if (accept(leaf) && remove(leaf))
			{
				--(*size);
				delete leaf;
				return true;
			}
			
			return false;
		}
	};
	
	
	template <typename Acceptor, typename LeafRemover>
	struct RemoveFunctor :
		std::unary_function< const BoundedItem *, bool > 
	{
		const Acceptor &accept;
		LeafRemover &remove;
		
		// parameters that are passed in
		std::list<Leaf*> * itemsToReinsert;
		std::size_t * m_size;
	
		// the third parameter is a list that the items that need to be reinserted
		// are put into
		explicit RemoveFunctor(const Acceptor &na, LeafRemover &lr, std::list<Leaf*>* ir, std::size_t * size)
			: accept(na), remove(lr), itemsToReinsert(ir), m_size(size) {}
	
		bool operator()(BoundedItem * item, bool isRoot = false)
		{
			Node * node = static_cast<Node*>(item);
		
			if (accept(node))
			{	
				// this is the easy part: remove nodes if they need to be removed
				if (node->hasLeaves)
					node->items.erase(std::remove_if(node->items.begin(), node->items.end(), RemoveLeafFunctor<Acceptor, LeafRemover>(accept, remove, m_size)), node->items.end());
				else
					node->items.erase(std::remove_if(node->items.begin(), node->items.end(), *this), node->items.end() );

				if (!isRoot)
				{
					if (node->items.empty())
					{
						// tell parent to remove us if theres nothing left
						delete node;
						return true;
					}
					else if (node->items.size() < min_child_items)
					{
						// queue up the items that need to be reinserted
						QueueItemsToReinsert(node);
						return true;
					}
				}
				else if (node->items.empty())
				{
					// if the root node is empty, setting these won't hurt
					// anything, since the algorithms don't actually require 
					// the nodes to have anything in them. 
					node->hasLeaves = true;
					node->bound.reset();
				}
			}			
			
			// anything else, don't remove it
			return false;
			
		}
		
		// theres probably a better way to do this, but this
		// traverses and finds any leaves, and adds them to a
		// list of items that will later be reinserted
		void QueueItemsToReinsert(Node * node)
		{
			typename std::vector< BoundedItem* >::iterator it = node->items.begin();
			typename std::vector< BoundedItem* >::iterator end = node->items.end();
		
			if (node->hasLeaves)
			{
				for(; it != end; it++)
					itemsToReinsert->push_back(static_cast<Leaf*>(*it));
			}
			else
				for (; it != end; it++)
					QueueItemsToReinsert(static_cast<Node*>(*it));
					
			delete node;
		}
	}; */

	//template <typename RSTarNode>
	struct SortNodesByFirstEdge : 
		public std::binary_function< const AABox * const, const AABox * const, bool >
	{
		const int m_axis;
		explicit SortNodesByFirstEdge (int axis) : m_axis(axis) {}
		
		bool operator() (const AABox * const bi1, const AABox * const bi2) const 
		{
			return bi1->low()[m_axis] < bi2->low()[m_axis];
		}
	};
	
//	template <typename RSTarNode>
	struct SortNodesBySecondEdge : 
		public std::binary_function< const AABox * const, const AABox * const, bool >
	{
		int m_axis;
		explicit SortNodesBySecondEdge (int axis) : m_axis(axis) {}
	
		bool operator() (const AABox * const bi1, const AABox * const bi2) const 
		{
			return bi1->high()[m_axis] < bi2->high()[m_axis];
		}
	};
	
	
//	template <typename RSTarNode>
	struct SortNodesByDistanceFromCenter : 
		public std::binary_function< const AABox * const, const AABox * const, bool >
	{
		const AABox * const m_center;
		explicit SortNodesByDistanceFromCenter(const AABox * const center) : m_center(center) {}
	
		bool operator() (const AABox * const bi1, const AABox * const bi2) const 
		{
			return (bi1->center() - m_center->center()).length() < (bi2->center() - m_center->center()).length();
		}
	};
	
	//template <typename AABox>
	struct SortNodesByAreaEnlargement : 
		public std::binary_function< const AABox * const, const AABox * const, bool >
	{
		const float area;
		explicit SortNodesByAreaEnlargement(const AABox * center) : area(center->area()) {}
	
		bool operator() (const AABox * const bi1, const AABox * const bi2) const 
		{
			return area - bi1->area() < area - bi2->area();
		}
	};
	
//	template <typename AABox>
	struct SortNodesByOverlapEnlargement : 
		public std::binary_function< const AABox * const, const AABox * const, bool >
	{
		const AABox * const m_center;
		explicit SortNodesByOverlapEnlargement(const AABox * const center) : m_center(center) {}
	
		bool operator() (const AABox * const bi1, const AABox * const bi2) const 
		{
			return bi1->intersect(*m_center).area() < bi2->intersect(*m_center).area();
		}
	};
	
	/**********************************************************
	 * Functor used to iterate over a set and stretch a
	 * AABox
	 **********************************************************/
	
//	template <typename RSTarNode>
	struct StretchAABox : 
		public std::unary_function< const AABox * const, void >
	{
		AABox * m_bound;
		explicit StretchAABox(AABox * bound) : m_bound(bound) {}
	
		void operator() (const AABox * const item)
		{
			m_bound->merge(*item);
		}
	};
};

#endif /* RSTARTREE_H */
