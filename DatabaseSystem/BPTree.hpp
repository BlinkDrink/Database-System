#pragma once
#include <iostream>
#include<vector>
#include<set>
#include "RecordPtr.hpp"
#include "TypeWrapper.hpp"
#include "Query.hpp"

using std::set;
using std::pair;
using std::vector;
using data = pair<TypeWrapper, RecordPtr>;

#define DEFAULT_ORDER 5

// BP node
class Node {
public:
	bool fIsLeaf;
	int fOrder;
	vector<data> fKeys;
	vector<Node*> ptr;
	//friend class BPTree;

public:
	Node(int order, bool isLeaf) : fIsLeaf(isLeaf), fOrder(order)
	{
		for (size_t i = 0; i < order + 1; i++)
			ptr.push_back(nullptr);
	}

	int keyIndex(const TypeWrapper& key)
	{
		for (int i = 0; i < fKeys.size(); i++)
			if (fKeys[i].first == key)
				return i;

		return -1;
	}
};

// BP tree
class BPTree {
public:
	BPTree() : root(nullptr), fOrder(DEFAULT_ORDER), fSize(0) {}

	BPTree(int order) : root(nullptr), fOrder(order), fSize(0) {}

	BPTree(const BPTree& other)
	{
		this->root = copy(other.root);
		this->fOrder = other.fOrder;
		this->fSize = other.fSize;
	}

	BPTree& operator=(const BPTree& other)
	{
		if (this != &other)
		{
			clear(root);
			if (other.fSize != 0)
				root = copy(other.root);
		}

		return *this;
	}

	BPTree(BPTree&& other) noexcept : BPTree()
	{
		std::swap(fOrder, other.fOrder);
		std::swap(fSize, other.fSize);
		std::swap(root, other.root);
	}

	BPTree& operator=(BPTree&& other) noexcept
	{
		if (this != &other)
		{
			std::swap(fOrder, other.fOrder);
			std::swap(fSize, other.fSize);
			std::swap(root, other.root);
		}

		return *this;
	}

	BPTree(ifstream& in)
	{
		root = nullptr;
		in.read((char*)&fOrder, sizeof(fOrder));
		in.read((char*)&fSize, sizeof(fSize));
		size_t tmpSize = fSize;
		for (size_t i = 0; i < tmpSize; i++)
			this->insert({ TypeWrapper(in), RecordPtr(in) });

		fSize = tmpSize;
	}

	~BPTree() { clear(root); fSize = 0; }

	RecordPtr getRecordAtIndex(const TypeWrapper& key)
	{
		Node* target = search(key);
		return target->fKeys[target->keyIndex(key)].second;
	}

	/**
	 * @brief Searches the tree for given key
	 * @param key - key to be searched for
	 * @return the node containing the key, nullptr otherwise
	*/
	Node* search(const TypeWrapper& key)
	{
		if (root == nullptr)
			return nullptr;

		Node* cursor = root;
		while (cursor->fIsLeaf == false)
		{
			for (int i = 0; i < cursor->fKeys.size(); i++)
			{
				if (key < cursor->fKeys[i].first)
				{
					cursor = cursor->ptr[i];
					break;
				}

				if (i == cursor->fKeys.size() - 1)
				{
					cursor = cursor->ptr[i + 1];
					break;
				}
			}
		}

		for (int i = 0; i < cursor->fKeys.size(); i++)
			if (cursor->fKeys[i].first == key)
				return cursor;

		return nullptr;
	}

	/**
	 *	@brief Insert function. By given key, find it's place among the leaf nodes, then insert it into the keys of the node.
	 *	If overflow occurs, split the node into two, take the median index of the split
	 *	and send it up the tree, then readjust the parent's child pointers.
	 *	After sending the median index upwards an overflow can occur,
	 *	so insertInternal is called once that happens and finds the place for the node recursively.
	 *	@param kvp - the value to be inserted in the tree
	*/
	void insert(data kvp)
	{
		if (root == nullptr)
		{
			root = new Node(fOrder, true);
			root->fKeys.push_back(kvp);
		}
		else
		{
			Node* cursor = root;
			Node* parent = nullptr;
			while (cursor->fIsLeaf == false)
			{
				parent = cursor;
				for (int i = 0; i < cursor->fKeys.size(); i++)
				{
					if (kvp.first < cursor->fKeys[i].first)
					{
						cursor = cursor->ptr[i];
						break;
					}

					if (i == cursor->fKeys.size() - 1)
					{
						cursor = cursor->ptr[i + 1];
						break;
					}
				}
			}

			if (cursor->fKeys.size() < fOrder)
			{
				size_t pos = 0;
				while (pos < cursor->fKeys.size() && kvp.first > cursor->fKeys[pos].first)
					pos++;

				cursor->fKeys.insert(cursor->fKeys.begin() + pos, kvp);
				cursor->ptr[cursor->fKeys.size()] = cursor->ptr[cursor->fKeys.size() - 1];
				cursor->ptr[cursor->fKeys.size() - 1] = nullptr;
			}
			else
			{
				Node* newLeaf = splitNode(cursor, kvp);

				if (cursor == root)
				{
					Node* newRoot = new Node(fOrder, false);
					newRoot->fKeys.push_back(newLeaf->fKeys.front());
					newRoot->ptr[0] = cursor;
					newRoot->ptr[1] = newLeaf;
					root = newRoot;
				}
				else
				{
					insertInternal(newLeaf->fKeys[0], parent, newLeaf);
				}
			}
		}
		fSize++;
	}

	/**
	 * @brief Remove function. By given key, find the corresponding node containing the key and delete the key from it.
	 * If underflow occurs, check if the leaf node's left or right sibling can lend a key from it's array of keys.
	 * If the sibling has >= (fOrder+1)/2 + 1 keys, then borrow one of them and stop, if not then merging of nodes takes place.
	 *		- when merging two nodes we take all the items from one node and pour them into the other.
	 *		- once merged, we need to adjust the parent's keys so that the properties of the B+ tree are kept.
	 *		- if an underflow occurs in the process of adjusting the parent's keys, then check the internal node's left/right sibling
	 *		  if it can spare a key - if yes then take that key and borrow it's corresponding child as well. If it cannot borrow any keys
	 *		  then merge leftNode, parent and right node.
	 * @param kvp - key to be removed
	*/
	void remove(const TypeWrapper& key)
	{
		if (!root)
			return;

		Node* cursor = root;
		Node* parent = nullptr;
		int leftSibling, rightSibling;
		// find the leaf node containing the kvp
		while (!cursor->fIsLeaf)
		{
			for (int i = 0; i < cursor->fKeys.size(); i++)
			{
				parent = cursor;
				leftSibling = i - 1;
				rightSibling = i + 1;
				if (key < cursor->fKeys[i].first)
				{
					cursor = cursor->ptr[i];
					break;
				}

				if (i == cursor->fKeys.size() - 1)
				{
					leftSibling = i;
					rightSibling = i + 2;
					cursor = cursor->ptr[i + 1];
					break;
				}
			}
		}

		// in the node, find the kvp, if it exists
		bool found = false;
		int pos;
		for (pos = 0; pos < cursor->fKeys.size(); pos++)
		{
			if (cursor->fKeys[pos].first == key)
			{
				found = true;
				break;
			}
		}

		if (!found)
			return;

		// erase the key from the node's keys
		cursor->fKeys.erase(cursor->fKeys.begin() + pos);
		fSize--;

		// in case we are deleting the only element in the tree, just delete the tree itself
		if (cursor == root)
		{
			for (int i = 0; i < fOrder + 1; i++)
				cursor->ptr[i] = nullptr;

			if (cursor->fKeys.size() == 0)
			{
				delete cursor;
				root = nullptr;
			}

			return;
		}

		// adjust the child pointers after deleting the element
		cursor->ptr[cursor->fKeys.size()] = cursor->ptr[cursor->fKeys.size() + 1];
		cursor->ptr[cursor->fKeys.size() + 1] = nullptr;

		if (cursor->fKeys.size() >= (fOrder + 1) / 2 - 1 /*delete -1*/)
		{
			deleteIndex(key, parent);
			return;
		}

		// Borrow
		if (leftSibling >= 0)
		{
			Node* leftNode = parent->ptr[leftSibling];
			if (leftNode->fKeys.size() >= (fOrder + 1) / 2 /*+ 1*/)
			{
				// take the last element from left sibling and insert it in cursor's start + readjust pointer to point to next leaf
				cursor->fKeys.insert(cursor->fKeys.begin(), leftNode->fKeys.back());
				cursor->ptr[cursor->fKeys.size()] = cursor->ptr[cursor->fKeys.size() - 1];
				cursor->ptr[cursor->fKeys.size() - 1] = nullptr;

				// after inserting last el. from leftSibling in cursor, erase it from left sibl. and readjust pointers
				leftNode->fKeys.erase(leftNode->fKeys.end() - 1);
				leftNode->ptr[leftNode->fKeys.size()] = cursor;
				leftNode->ptr[leftNode->fKeys.size() + 1] = nullptr;
				parent->fKeys[leftSibling] = cursor->fKeys[0];
				deleteIndex(key, parent);
				return;
			}
		}

		//Borrow
		if (rightSibling <= parent->fKeys.size())
		{
			Node* rightNode = parent->ptr[rightSibling];
			if (rightNode->fKeys.size() >= (fOrder + 1) / 2 /*+ 1*/)
			{
				// Borrow key from right sibling and readjust pointers
				cursor->fKeys.push_back(rightNode->fKeys[0]);
				cursor->ptr[cursor->fKeys.size()] = cursor->ptr[cursor->fKeys.size() - 1];
				cursor->ptr[cursor->fKeys.size() - 1] = nullptr;

				// Erase the borrowed element and readjust nodes
				rightNode->fKeys.erase(rightNode->fKeys.begin());
				rightNode->ptr[rightNode->fKeys.size()] = rightNode->ptr[rightNode->fKeys.size() + 1];
				rightNode->ptr[rightNode->fKeys.size() + 1] = nullptr;
				parent->fKeys[rightSibling - 1] = rightNode->fKeys[0]; // to fulfil the properties for b+tree, we take the smallest element
																	   // from the right sibling and put it in the parent's keys
				deleteIndex(key, parent);
				return;
			}
		}

		// Merges
		if (leftSibling >= 0)
		{
			Node* leftNode = parent->ptr[leftSibling];
			leftNode->ptr[leftNode->fKeys.size()] = nullptr;
			for (int j = 0; j < cursor->fKeys.size(); j++)
				leftNode->fKeys.push_back(cursor->fKeys[j]);

			leftNode->ptr[leftNode->fKeys.size()] = cursor->ptr[cursor->fKeys.size()];
			// Merging two leaf nodes
			removeInternal(parent->fKeys[leftSibling].first, parent, cursor);
			delete cursor;
		}
		else if (rightSibling <= parent->fKeys.size())
		{
			Node* rightNode = parent->ptr[rightSibling];
			cursor->ptr[cursor->fKeys.size()] = nullptr;
			for (int i = cursor->fKeys.size(), j = 0; j < rightNode->fKeys.size(); i++, j++)
				cursor->fKeys.insert(cursor->fKeys.begin() + i, rightNode->fKeys[j]);

			cursor->ptr[cursor->fKeys.size()] = rightNode->ptr[rightNode->fKeys.size()];
			// Merging two leaf nodes
			removeInternal(parent->fKeys[rightSibling - 1].first, parent, rightNode);
			delete rightNode;
		}

		deleteIndex(key, parent);
	}

	/**
	 * @brief !=
	*/
	vector<RecordPtr> getAllRecordPtrsExcept(const TypeWrapper& except)
	{
		vector<RecordPtr> answer;
		Node* cursor = root;
		while (cursor && !cursor->fIsLeaf)
			cursor = cursor->ptr[0];

		while (cursor)
		{
			for (int i = 0; i < cursor->fKeys.size(); i++)
			{
				if (cursor->fKeys[i].first == except)
					continue;

				answer.push_back(cursor->fKeys[i].second);
			}

			cursor = cursor->ptr[cursor->fKeys.size()];
		}

		return answer;
	}

	/**
	 * @brief > or >=
	*/
	vector<RecordPtr> getRecordPtrsGreaterThan(const TypeWrapper& what, bool orEqual)
	{
		vector<RecordPtr> answer;
		Node* cursor = root;
		while (cursor && !cursor->fIsLeaf)
		{
			for (size_t i = 0; i < cursor->fKeys.size(); i++)
			{
				if (cursor->fKeys[i].first > what)
				{
					cursor = cursor->ptr[i];
					break;
				}

				if (i == cursor->fKeys.size() - 1)
				{
					cursor = cursor->ptr[i + 1];
					break;
				}
			}
		}

		while (cursor)
		{
			for (int i = 0; i < cursor->fKeys.size(); i++)
			{
				if (orEqual && cursor->fKeys[i].first >= what)
					answer.push_back(cursor->fKeys[i].second);
				else if (!orEqual && cursor->fKeys[i].first > what)
					answer.push_back(cursor->fKeys[i].second);
			}

			cursor = cursor->ptr[cursor->fKeys.size()];
		}

		return answer;
	}

	/**
	 * @brief  < or <=
	*/
	vector<RecordPtr> getRecordPtrsLessThan(const TypeWrapper& what, bool orEqual)
	{
		vector<RecordPtr> answer;
		Node* cursor = root;
		while (cursor && !cursor->fIsLeaf)
			cursor = cursor->ptr[0];

		while (cursor)
		{
			for (int i = 0; i < cursor->fKeys.size(); i++)
			{
				if (orEqual && cursor->fKeys[i].first <= what)
					answer.push_back(cursor->fKeys[i].second);
				else if (!orEqual && cursor->fKeys[i].first < what)
					answer.push_back(cursor->fKeys[i].second);
			}

			cursor = cursor->ptr[cursor->fKeys.size()];
		}

		return answer;
	}

	/**
	 * @brief By given query with primary key get all the records satisfying its criteria
	 * @param query - data base query to check against tree's records
	 * @return array of record pointers that satisfy the criteria
	*/
	vector<RecordPtr> getRecordsFromQuery(InternalQuery& query)
	{
		if (!query.isPrimaryKeyQuery())
			throw invalid_argument("Cannot select/remove items from tree without primary index");

		vector<RecordPtr> answer;
		if (query.getOperator() == Operator::EQUAL)
		{
			Node* targetNode = search(query.getValue());
			if (targetNode)
				answer.push_back(targetNode->fKeys[targetNode->keyIndex(query.getValue())].second);
		}
		else if (query.getOperator() == Operator::NOT_EQUAL)
		{
			answer = getAllRecordPtrsExcept(query.getValue());
		}
		else if (query.getOperator() == Operator::GREATER_THAN)
		{
			answer = getRecordPtrsGreaterThan(query.getValue(), false);
		}
		else if (query.getOperator() == Operator::GREATER_THAN_OR_EQUAL)
		{
			answer = getRecordPtrsGreaterThan(query.getValue(), true);
		}
		else if (query.getOperator() == Operator::LESS_THAN)
		{
			answer = getRecordPtrsLessThan(query.getValue(), false);
		}
		else if (query.getOperator() == Operator::LESS_THAN_OR_EQUAL)
		{
			answer = getRecordPtrsLessThan(query.getValue(), true);
		}

		return answer;
	}

	Node* getRoot()
	{
		return root;
	}

	size_t size() const { return this->fSize; }

	/**
	 * @brief Write the tree to file, just the elements in root-left-right way
	 * @param out - output stream
	*/
	void write(ofstream& out)
	{
		set<TypeWrapper> visited;
		out.write((char*)&fOrder, sizeof(fOrder));
		out.write((char*)&fSize, sizeof(fSize));
		writeRec(root, out, visited);
	}

private:
	int fOrder;
	size_t fSize;
	Node* root;

	/**
	 * @brief Method used for inserting a kvp that is located neither in the leaves
	 * nor in the root.
	 * @param kvp - key value pair that will be inserted
	 * @param cursor - root of subtree
	 * @param child - child of cursor
	*/
	void insertInternal(data kvp, Node* cursor, Node* child)
	{
		/// Check to see if the current node can contain any more kvp's
		if (cursor->fKeys.size() < fOrder)
		{
			int i = 0;
			while (i < cursor->fKeys.size() && kvp.first > cursor->fKeys[i].first)
				i++;

			// Shift child pointers with 1 to the right
			for (int j = cursor->fKeys.size() + 1; j > i + 1; j--)
				cursor->ptr[j] = cursor->ptr[j - 1];

			cursor->fKeys.insert(cursor->fKeys.begin() + i, kvp);
			cursor->ptr[i + 1] = child;
		}
		else
		{
			Node* newInternal = new Node(fOrder, false); // create new internal node when splitting
			vector<data> virtualKvp;
			vector<Node*> virtualPtr;
			virtualKvp.reserve(fOrder + 1); // we assume the keys are full so we reserve exactly fOrder+1 so we can split accordingly to the newly added el.
			virtualPtr.reserve(fOrder + 2); // if keys are full of an internal node, that means ptr is also full so we reserve fOrder+2

			virtualKvp.insert(virtualKvp.begin(), cursor->fKeys.begin(), cursor->fKeys.end()); // fill with cursor's half
			virtualPtr.insert(virtualPtr.begin(), cursor->ptr.begin(), cursor->ptr.end()); // fill with cursor's half

			int i = 0;
			while (i < fOrder && kvp.first > virtualKvp[i].first) // find the place for the new key
				i++;

			virtualKvp.insert(virtualKvp.begin() + i, kvp); // insert new key
			virtualPtr.insert(virtualPtr.begin() + i + 1, child); // insert the element from the previous iteration

			// We are halving the keys and pointers of cursor, because we are splitting it
			int k = cursor->ptr.size() - 1;
			while (cursor->fKeys.size() != (fOrder + 1) / 2)
			{
				cursor->fKeys.pop_back();
				cursor->ptr[k] = nullptr;
				k--;
			}

			// Since we dont need repeating elements in the internal nodes, we skip the first key here by saying fOrder + 1 - (fOrder+1)/2 - 1
			size_t newInternalKeysSize = fOrder - (fOrder + 1) / 2;
			newInternal->fKeys.reserve(newInternalKeysSize);
			newInternal->fKeys.insert(newInternal->fKeys.begin(), virtualKvp.begin() + cursor->fKeys.size() + 1, virtualKvp.end()); // Fill keys of newInternal
			for (size_t j = 0, i = cursor->fKeys.size() + 1; i < virtualPtr.size(); i++, j++)
				newInternal->ptr[j] = virtualPtr[i];

			if (cursor == root)
			{
				Node* newRoot = new Node(fOrder, false);
				newRoot->fKeys.push_back(virtualKvp[cursor->fKeys.size()]);
				newRoot->ptr[0] = cursor;
				newRoot->ptr[1] = newInternal;
				root = newRoot;
			}
			else
			{
				insertInternal(virtualKvp[cursor->fKeys.size()], findParent(root, cursor), newInternal);
			}
		}
	}

	/**
	 * @brief Given root of tree and a child node, find the parent of that child node
	 * @param root - begining of the subtree
	 * @param child - child whoose parent we will look for
	 * @return the parent if found, otherwise nullptr
	*/
	Node* findParent(Node* root, Node* child)
	{
		if (child == root || root == nullptr)
			return nullptr;

		Node* parent = nullptr;
		if (root->fIsLeaf || (root->ptr[0])->fIsLeaf)
			return nullptr;

		for (Node* rootChild : root->ptr)
		{
			if (rootChild == child)
			{
				return root;
			}
			else
			{
				parent = findParent(rootChild, child);
				if (parent)
					return parent;
			}
		}

		return parent;
	}

	/**
	 * @brief Used in insert(). By given node, the function splits the node into two, returning the newly created leaf(always right of cursor)
	 * @param cursor - node which will be splited into two
	 * @param kvp - key that triggers the split of a node (node can have maximum fOrder elements, so having fOrder+1 keys trigers a split)
	 * @return the right half of the splited node(dynamically allocated)
	*/
	Node* splitNode(Node*& cursor, data& kvp)
	{
		Node* newLeaf = new Node(fOrder, true);
		vector<data> virtualNode;
		virtualNode.reserve(fOrder + 1);
		virtualNode.insert(virtualNode.begin(), cursor->fKeys.begin(), cursor->fKeys.end());

		int i = 0;
		while (i < fOrder && kvp.first > virtualNode[i].first)
			i++;
		virtualNode.insert(virtualNode.begin() + i, kvp);

		cursor->fKeys.clear();
		cursor->fKeys.reserve((fOrder + 1) / 2);
		cursor->fKeys.insert(cursor->fKeys.begin(), virtualNode.begin(), virtualNode.begin() + (fOrder + 1) / 2);

		size_t newLeafKeysSize = fOrder + 1 - (fOrder + 1) / 2;
		newLeaf->fKeys.reserve(newLeafKeysSize);
		newLeaf->fKeys.insert(newLeaf->fKeys.begin(), virtualNode.begin() + (fOrder + 1) / 2, virtualNode.end());

		cursor->ptr[cursor->fKeys.size()] = newLeaf;
		newLeaf->ptr[newLeaf->fKeys.size()] = cursor->ptr[fOrder];
		cursor->ptr[fOrder] = nullptr;

		return newLeaf;
	}

	/**
	 * @brief Clears the B+ tree
	 * @param cursor - root from which clearing takes place
	*/
	void clear(Node*& cursor)
	{
		if (cursor != nullptr)
		{
			if (!cursor->fIsLeaf)
			{
				for (Node*& child : cursor->ptr)
				{
					clear(child);
				}
			}

			delete cursor;
			cursor = nullptr;
		}
	}

	/**
	 * @brief Used in remove(). This function is called in case the key we are deleting trigers merge process for nodes.
	 * This usually happens when after deleting the desired key we are left with keys < (fOrder/2) keys (the minimum of the B+ tree property)
	 * @param kvp - key that will be removed in the recursive call to remove internal
	 * @param cursor - parent of child
	 * @param child - child node(result of the merging of nodes). It is a leftover from the merge process with it's right or left brother and will be deleted
	*/
	void removeInternal(const TypeWrapper& kvp, Node*& cursor, Node*& child) {
		if (cursor == root)
		{
			if (cursor->fKeys.size() == 1)
			{
				if (cursor->ptr[1] == child)
				{
					// Changing root node
					delete child;
					root = cursor->ptr[0];
					delete cursor;
					cursor = child = nullptr;
					return;
				}
				else if (cursor->ptr[0] == child) {
					// Changing root node
					delete child;
					root = cursor->ptr[1];
					delete cursor;
					cursor = child = nullptr;
					return;
				}
			}
		}

		int pos;
		// find pos of child that is to be deleted
		for (pos = 0; pos < cursor->fKeys.size() + 1; pos++)
			if (cursor->ptr[pos] == child)
				break;

		// shift pointers one to the left so we "eat" the child that is to be deleted
		for (int i = pos; i < cursor->fKeys.size() + 1; i++)
		{
			if (i != cursor->fKeys.size())
				cursor->ptr[i] = cursor->ptr[i + 1];
			else
				cursor->ptr[i] = nullptr;
		}

		for (pos = 0; pos < cursor->fKeys.size(); pos++)
			if (cursor->fKeys[pos].first == kvp)
				break;

		// Erase the key that we have sent up the tree
		cursor->fKeys.erase(cursor->fKeys.begin() + pos);

		if (cursor->fKeys.size() >= (fOrder + 1) / 2 - 1)
			return;

		if (cursor == root)
			return;

		Node* parent = findParent(root, cursor);
		int leftSibling = -1, rightSibling = parent->fKeys.size() + 1;
		for (pos = 0; pos < parent->fKeys.size() + 1; pos++)
		{
			if (parent->ptr[pos] == cursor)
			{
				leftSibling = pos - 1;
				rightSibling = pos + 1;
				break;
			}
		}

		if (leftSibling >= 0)
		{
			Node* leftNode = parent->ptr[leftSibling];
			if (leftNode->fKeys.size() >= (fOrder + 1) / 2)
			{
				cursor->fKeys.insert(cursor->fKeys.begin(), parent->fKeys[leftSibling]);
				parent->fKeys[leftSibling] = leftNode->fKeys[leftNode->fKeys.size() - 1];
				for (int i = cursor->fKeys.size() + 1; i > 0; i--)
					cursor->ptr[i] = cursor->ptr[i - 1];

				cursor->ptr[0] = leftNode->ptr[leftNode->fKeys.size()];
				leftNode->ptr[leftNode->fKeys.size()] = nullptr;
				leftNode->fKeys.pop_back();
				return;
			}
		}

		if (rightSibling <= parent->fKeys.size())
		{
			Node* rightNode = parent->ptr[rightSibling];
			if (rightNode->fKeys.size() >= (fOrder + 1) / 2)
			{
				cursor->fKeys.push_back(parent->fKeys[pos]);
				parent->fKeys[pos] = rightNode->fKeys[0];
				cursor->ptr[cursor->fKeys.size()/*delete +1*/] = rightNode->ptr[0];

				for (int i = 0; i < rightNode->fKeys.size(); ++i)
					rightNode->ptr[i] = rightNode->ptr[i + 1];

				rightNode->ptr[rightNode->fKeys.size()] = nullptr;
				rightNode->fKeys.erase(rightNode->fKeys.begin());
				return;
			}
		}

		if (leftSibling >= 0)
		{
			Node* leftNode = parent->ptr[leftSibling];
			leftNode->fKeys.push_back(parent->fKeys[leftSibling]);

			for (int i = leftNode->fKeys.size(), j = 0; i < fOrder + 1 && j < cursor->fKeys.size() + 1; j++, i++)
			{
				leftNode->ptr[i] = cursor->ptr[j];
				cursor->ptr[j] = nullptr;
			}

			for (int j = 0; j < cursor->fKeys.size(); j++)
				leftNode->fKeys.push_back(cursor->fKeys[j]);

			removeInternal(parent->fKeys[leftSibling].first, parent, cursor);
		}
		else if (rightSibling <= parent->fKeys.size())
		{
			Node* rightNode = parent->ptr[rightSibling];
			cursor->fKeys.push_back(parent->fKeys[rightSibling - 1]);

			for (int i = cursor->fKeys.size(), j = 0; i < fOrder + 1 && j < rightNode->fKeys.size() + 1; j++, i++)
			{
				cursor->ptr[i] = rightNode->ptr[j];
				rightNode->ptr[j] = nullptr;
			}

			for (int j = 0; j < rightNode->fKeys.size(); j++)
				cursor->fKeys.push_back(rightNode->fKeys[j]);

			removeInternal(parent->fKeys[rightSibling - 1].first, parent, rightNode);
		}
	}

	/**
	 * @brief Called after remove(), used to delete the second occurence of an index (if second occurence exists)
	 * @param kvp - key to delete
	 * @param cursor - pivot element from which searching element for deletion takes place
	*/
	void deleteIndex(const TypeWrapper& key, Node* cursor)
	{
		if (!cursor)
			return;

		int indKvp = cursor->keyIndex(key);
		if (indKvp == -1)
			deleteIndex(key, findParent(root, cursor));
		else
			cursor->fKeys[indKvp] = getSmallestElementInSubTree(cursor->ptr[indKvp + 1]);
	}

	/**
	 * @brief By given pivot point {root} find the smallest index in its subtree
	 * @param root - pivot node
	 * @return the smallest index in the subtree of root
	*/
	data getSmallestElementInSubTree(Node* root)
	{
		while (!root->fIsLeaf)
			root = root->ptr[0];

		if (!root)
			return { -1,{-1,-1} };

		return root->fKeys.front();
	}

	/**
	 * @brief Copy the tree given by it's root recursively
	 * @param root - tree to be copied
	 * @return Newly copied tree
	*/
	Node* copy(Node* root)
	{
		if (!root)
			return nullptr;

		if (root->fIsLeaf)
		{
			Node* leaf = new Node(root->fOrder, root->fIsLeaf);
			for (data& val : root->fKeys)
				leaf->fKeys.push_back(val);

			for (int i = 0; i < root->fKeys.size() + 1; i++)
				leaf->ptr[i] = root->ptr[i];

			return leaf;
		}
		else
		{
			Node* inner = new Node(root->fOrder, root->fIsLeaf);

			for (data& val : root->fKeys)
				inner->fKeys.push_back(val);

			for (unsigned short slot = 0; slot < root->fKeys.size() + 1; ++slot)
				inner->ptr[slot] = copy(root->ptr[slot]);

			return inner;
		}
	}

	/**
	 * @brief Used in writing the tree to file
	 * @param cursor - begining of the subTree
	 * @param out - output stream
	 * @param visitedKeys - set of all the keys that have been written already
	*/
	void writeRec(Node* cursor, ofstream& out, set<TypeWrapper>& visitedKeys)
	{
		if (cursor) {
			for (int i = 0; i < cursor->fKeys.size(); i++)
			{
				if (visitedKeys.find(cursor->fKeys[i].first) != visitedKeys.end())
					continue;

				cursor->fKeys[i].first.write(out);
				cursor->fKeys[i].second.write(out);
				visitedKeys.insert(cursor->fKeys[i].first);
			}

			if (!cursor->fIsLeaf)
				for (int i = 0; i < cursor->fKeys.size() + 1; i++)
					writeRec(cursor->ptr[i], out, visitedKeys);
		}
	}

};