/*  Copyright (C) 2010-2011  kaosu (qiupf2000@gmail.com)
 *  This file is part of the Interactive Text Hooker.

 *  Interactive Text Hooker is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#define STACK_SIZE 32
#ifndef ITH_STACK
#define ITH_STACK
template<class T, size_t stack_size>
class MyStack
{
public:
	MyStack(): index(0) {}
	void push_back(const T& e)
	{
		if (index<stack_size)
		s[index++]=e;
	}
	void pop_back()
	{
		index--;
	}
	T& back()
	{
		return s[index-1];
	}
	T& operator[](size_t i) {return s[i];}
	size_t size() {return index;}
private:
	size_t index;
	T s[stack_size];
};
#endif
template <class T, class D>
class TreeNode
{
public:
	TreeNode():key(),data(),Left(0),Right(0),Parent(0),rank(1),factor(0) {}
	TreeNode(const T& k, const D& d)
		:key(k),data(d),Left(0),Right(0),Parent(0),rank(1),factor(0) {}
	TreeNode* Successor() 
	{
		TreeNode* Node,*ParentNode;
		Node=Right;
		if (Node==0)
		{
			Node=this;
			for (;;)
			{
				ParentNode=Node->Parent;
				if (ParentNode==0) return 0;
				if (ParentNode->Left==Node) break;
				Node=ParentNode;
			}
			return ParentNode;
		}
		else
			while (Node->Left) Node=Node->Left;
		return Node;
	}
	TreeNode* Predecessor()
	{
		TreeNode* Node,ParentNode;
		Node=Left;
		if (Node==0)
		{
			Node=this;
			for(;;)
			{
				ParentNode=Node->Parent;
				if (ParentNode==0) return 0;
				if (ParentNode->Right==Node) break;
				Node=ParentNode;
			}
			return ParentNode;
		}
		else
			while (Node->Right) Node=Node->Right;
		return Node;
	}
	TreeNode *Left,*Right,*Parent;
	size_t rank;
	__int64 factor;
	T key;
	D data;
};
template<class T,class D>
class NodePath
{
public:
	NodePath(TreeNode<T,D> *n=0,__int64 f=0):Node(n),factor(f) {}
	TreeNode<T,D> *Node;
	__int64 factor;
};

template <class T,class D, class fComp, class fCopy, class fLength>
class AVLTree
{
public:
	AVLTree() {}
	~AVLTree()
	{
		while (head.Left)
			DeleteRoot();
	}
	TreeNode<T*,D>* TreeRoot() const {return head.Left;}
	TreeNode<T*,D>* Insert(T* key, const D& data)
	{
		if (head.Left)
		{
			MyStack<TreeNode<T*,D>*,STACK_SIZE> path; 
			TreeNode<T*,D> *DownNode,*ParentNode,*BalanceNode,*TryNode,*NewNode; //P,T,S,Q
			ParentNode=&head;
			path.push_back(ParentNode);
			__int64 factor,f;
			BalanceNode=DownNode=head.Left;
			for(;;) //The first part of AVL tree insert. Just do as binary tree insert routine and record some nodes.
			{
				factor = fCmp(key,DownNode->key);
				if (factor == 0 ) return DownNode; //Duplicate key. Return and do nothing.
				TryNode = _FactorLink(DownNode,factor);
				if (factor==-1) path.push_back(DownNode);
				if (TryNode) //DownNode has a child.
				{
					if (TryNode->factor!=0) //Keep track of unbalance node and its parent.
					{
						ParentNode=DownNode;
						BalanceNode=TryNode;
					}	
					DownNode=TryNode;
				}
				else break; //Finished binary tree search;
			}
			while(path.size())
			{
				path.back()->rank++;
				path.pop_back();
			}
			T* new_key=new T[fLen(key)+1];
			fCpy(new_key,key);
			TryNode=new TreeNode<T*,D>(new_key,data);
			_FactorLink(DownNode,factor) = TryNode;
			TryNode->Parent = DownNode;
			NewNode=TryNode;
			//Finished binary tree insert. Next to do is modify balance factors between 
			//BalanceNode and the new node.
			TreeNode<T*,D>* ModifyNode;
			factor=fCmp(key,BalanceNode->key);
			//factor=key<BalanceNode->key ? factor=-1:1; //Determine the balance factor at BalanceNode.
			ModifyNode=DownNode=_FactorLink(BalanceNode,factor); 
			//ModifyNode will be the 1st child.
			//DownNode will travel from here to the recent inserted node (TryNode).
			while(DownNode!=TryNode) //Check if we reach the bottom.
			{
				f=fCmp(key,DownNode->key);
				//f=_FactorCompare(key,DownNode->key);
				DownNode->factor=f;
				DownNode = _FactorLink(DownNode,f);//Modify balance factor and travels down.
			}
			//Finshed modifying balance factor.
			//Next to do is check the tree if it's unbalance and recover balance.
			if (BalanceNode->factor==0)  //Tree has grown higher. 
			{
				BalanceNode->factor=factor;
				_IncreaseHeight(); //Modify balance factor and increase the height.
				return NewNode;
			}
			if (BalanceNode->factor+factor==0) //Tree has gotten more balanced.
			{
				BalanceNode->factor=0; //Set balance factor to 0.
				return NewNode;
			}
			//Tree has gotten out of balance.
			if (ModifyNode->factor==factor) //A node and its child has same factor. Single rotation.
				DownNode = _SingleRotation(BalanceNode,ModifyNode, factor);	
			else //A node and its child has converse factor. Double rotation.
				DownNode = _DoubleRotation(BalanceNode, ModifyNode, factor);
			//Finished the balancing work. Set child field to the root of the new child tree.
			if (BalanceNode==ParentNode->Left) ParentNode->Left=DownNode;
			else ParentNode->Right=DownNode;
			return NewNode;
		}
		else //root null?
		{
			T* new_key=new T[fLen(key)+1];
			fCpy(new_key,key);
			head.Left=new TreeNode<T*,D>(new_key,data);
			head.rank++;
			_IncreaseHeight();
			return head.Left;
		}
	}
	bool Delete(T* key)
	{
		NodePath<T*,D> PathNode;
		MyStack<NodePath<T*,D>,STACK_SIZE> path; //Use to record a path to the destination node.
		path.push_back(NodePath<T*,D>(&head,-1));
		TreeNode<T*,D> *TryNode,*ChildNode,*BalanceNode,*SuccNode; 
		TryNode=head.Left;
		__int64 factor;
		while (1) //Search for the 
		{
			if (TryNode==0) return false; //Not found.
			factor = fCmp(key,TryNode->key);
			if (factor==0) break; //Key found, continue to delete.
			//factor = _FactorCompare( key, TryNode->key );
			path.push_back(NodePath<T*,D>(TryNode,factor));
			TryNode=_FactorLink(TryNode,factor); //Move to left.
		}
		SuccNode=TryNode->Right; //Find a successor.
		factor=1;
		if (SuccNode==0) 
		{
			SuccNode=TryNode->Left;
			factor=-1;
		}
		path.push_back(NodePath<T*,D>(TryNode,factor));
		while (SuccNode)
		{
			path.push_back(NodePath<T*,D>(SuccNode,-factor));
			SuccNode=_FactorLink(SuccNode,-factor);
		}
		PathNode=path.back();
		delete TryNode->key;
		TryNode->key=PathNode.Node->key; //Replace key and data field with the successor or predecessor.
		PathNode.Node->key=0;
		TryNode->data=PathNode.Node->data;
		path.pop_back();
		_FactorLink(path.back().Node,path.back().factor) = _FactorLink(PathNode.Node,-PathNode.factor); 
		delete PathNode.Node; //Remove the successor from the tree and release memory.
		PathNode=path.back();
		for (size_t i=0;i<path.size();i++)
			if (path[i].factor==-1) path[i].Node->rank--;
		while (1) //Rebalance the tree along the path back to the root.
		{
			if (path.size()==1)
			{
				_DecreaseHeight(); break;
			}
			BalanceNode=PathNode.Node;
			if (BalanceNode->factor==0) 
				//A balance node, just need to adjust the factor. Don't have to recurve since subtree height stays.
			{
				BalanceNode->factor=-PathNode.factor;
				break;
			}
			if (BalanceNode->factor==PathNode.factor) //Node get more balance. Subtree height decrease, need to recurve.
			{
				BalanceNode->factor=0;
				path.pop_back();
				PathNode=path.back();
				continue;
			}
			//Node get out of balance. Here raises 3 cases.
			ChildNode = _FactorLink(BalanceNode, -PathNode.factor);
			if (ChildNode->factor == 0) //New case different to insert operation.
			{
				TryNode = _SingleRotation2( BalanceNode, ChildNode, BalanceNode->factor );
				path.pop_back();
				PathNode=path.back();
				_FactorLink(PathNode.Node, PathNode.factor) = TryNode;
				break;
			}
			else
			{
				if ( ChildNode->factor == BalanceNode->factor ) //Analogous to insert operation case 1.
					TryNode = _SingleRotation( BalanceNode, ChildNode, BalanceNode->factor );
				else if ( ChildNode->factor + BalanceNode->factor == 0 ) //Analogous to insert operation case 2.
					TryNode = _DoubleRotation( BalanceNode, ChildNode, BalanceNode->factor );
			}
			path.pop_back(); //Recurve back along the path.
			PathNode=path.back();
			_FactorLink(PathNode.Node, PathNode.factor) = TryNode;
		}
		return true;
	}
	D& operator [] (T* key)
	{
		return (Insert(key,D())->data);
	}
	TreeNode<T*,D>* Search(T* key)
	{
		TreeNode<T*,D>* Find=head.Left;
		size_t k;
		while (Find!=0&&Find->key!=key)
		{
			k=fCmp(key, Find->key);
			if (k==0) break;
			Find = _FactorLink(Find, k);
		}
		return Find;
	}
	TreeNode<T*,D>* SearchIndex(size_t rank)
	{
		size_t r = head.rank;
		if (++rank>=r) return 0;
		TreeNode<T*,D>* n=&head;
		while (r!=rank)
		{
			if (rank>r)
			{
				n=n->Right;
				rank-=r;
				r=n->rank;
			}
			else
			{
				n=n->Left;
				r=n->rank;
			}
		}
		return n;
	}
	TreeNode<T*,D>* Begin()
	{
		TreeNode<T*,D>* Node=head.Left;
		if (Node)
			while (Node->Left) Node=Node->Left;
		return Node;
	}
	TreeNode<T*,D>* End()
	{
		TreeNode<T*,D>* Node=head.Left;
		if (Node)
			while (Node->Right) Node=Node->Right;
		return Node;
	}
	size_t Count() const {return head.rank-1;}
	template <class Fn>
	Fn TraverseTree(Fn& f)
	{
		return TraverseTreeNode(head.Left,f);
	}
protected:
	bool DeleteRoot()
	{
		NodePath<T*,D> PathNode;
		MyStack<NodePath<T*,D>,STACK_SIZE> path; //Use to record a path to the destination node.
		path.push_back(NodePath<T*,D>(&head,-1));
		TreeNode<T*,D> *TryNode,*ChildNode,*BalanceNode,*SuccNode; 
		TryNode=head.Left;
		__int64 factor;
		SuccNode=TryNode->Right; //Find a successor.
		factor=1;
		if (SuccNode==0) 
		{
			SuccNode=TryNode->Left;
			factor=-1;
		}
		path.push_back(NodePath<T*,D>(TryNode,factor));
		while (SuccNode)
		{
			path.push_back(NodePath<T*,D>(SuccNode,-factor));
			SuccNode=_FactorLink(SuccNode,-factor);
		}
		PathNode=path.back();
		delete[] TryNode->key;
		TryNode->key=PathNode.Node->key; //Replace key and data field with the successor.
		PathNode.Node->key=0;
		TryNode->data=PathNode.Node->data;
		path.pop_back();
		_FactorLink(path.back().Node,path.back().factor) = _FactorLink(PathNode.Node,-PathNode.factor); 
		delete PathNode.Node; //Remove the successor from the tree and release memory.
		PathNode=path.back();
		for (size_t i=0;i<path.size();i++)
			if (path[i].factor==-1) path[i].Node->rank--;
		while (1) //Rebalance the tree along the path back to the root.
		{
			if (path.size()==1)
			{
				_DecreaseHeight(); break;
			}
			BalanceNode=PathNode.Node;
			if (BalanceNode->factor==0) 
				//A balance node, just need to adjust the factor. Don't have to recurve since subtree height stays.
			{
				BalanceNode->factor=-PathNode.factor;
				break;
			}
			if (BalanceNode->factor==PathNode.factor) //Node get more balance. Subtree height decrease, need to recurve.
			{
				BalanceNode->factor=0;
				path.pop_back();
				PathNode=path.back();
				continue;
			}
			//Node get out of balance. Here raises 3 cases.
			ChildNode = _FactorLink(BalanceNode, -PathNode.factor);
			if (ChildNode->factor == 0) //New case different to insert operation.
			{
				TryNode = _SingleRotation2( BalanceNode, ChildNode, BalanceNode->factor );
				path.pop_back();
				PathNode=path.back();
				_FactorLink(PathNode.Node, PathNode.factor) = TryNode;
				break;
			}
			else
			{
				if ( ChildNode->factor == BalanceNode->factor ) //Analogous to insert operation case 1.
					TryNode = _SingleRotation( BalanceNode, ChildNode, BalanceNode->factor );
				else if ( ChildNode->factor + BalanceNode->factor == 0 ) //Analogous to insert operation case 2.
					TryNode = _DoubleRotation( BalanceNode, ChildNode, BalanceNode->factor );
			}
			path.pop_back(); //Recurve back along the path.
			PathNode=path.back();
			_FactorLink(PathNode.Node, PathNode.factor) = TryNode;
		}
		return true;
	}
	template <class Fn>
	Fn TraverseTreeNode(TreeNode<T*,D>* Node, Fn& f)
	{
		if (Node)
		{
			if (Node->Left) TraverseTreeNode(Node->Left,f);
			f(Node);
			if (Node->Right) TraverseTreeNode(Node->Right,f);
		}
		return f;	
	}
	inline TreeNode<T*,D>* _SingleRotation(TreeNode<T*,D>* BalanceNode, TreeNode<T*,D>* ModifyNode, __int64 factor)
	{
		TreeNode<T*,D>* Node = _FactorLink(ModifyNode, -factor);
		_FactorLink(BalanceNode, factor) = Node;
		_FactorLink(ModifyNode, -factor) = BalanceNode;
		if (Node) Node->Parent = BalanceNode;
		ModifyNode->Parent = BalanceNode->Parent;
		BalanceNode->Parent = ModifyNode;
		BalanceNode->factor = ModifyNode->factor = 0; //After single rotation, set all factor of 3 node to 0.
		if (factor==1) ModifyNode->rank+=BalanceNode->rank;
		else BalanceNode->rank-=ModifyNode->rank;
		return ModifyNode;
	}
	inline TreeNode<T*,D>* _SingleRotation2(TreeNode<T*,D>* BalanceNode, TreeNode<T*,D>* ModifyNode, __int64 factor)
	{
		TreeNode<T*,D>* Node = _FactorLink(ModifyNode, -factor);
		_FactorLink(BalanceNode, factor) = Node;
		_FactorLink(ModifyNode, -factor) = BalanceNode;
		if (Node) Node->Parent = BalanceNode;
		ModifyNode->Parent = BalanceNode->Parent;
		BalanceNode->Parent = ModifyNode;
		ModifyNode->factor = -factor;
		if (factor==1) ModifyNode->rank+=BalanceNode->rank;
		else BalanceNode->rank-=ModifyNode->rank;
		return ModifyNode;
	}
	inline TreeNode<T*,D>* _DoubleRotation(TreeNode<T*,D>* BalanceNode, TreeNode<T*,D>* ModifyNode, __int64 factor)
	{
		TreeNode<T*,D>* DownNode = _FactorLink(ModifyNode, -factor);
		TreeNode<T*,D>* Node1, *Node2;
		Node1 = _FactorLink(DownNode, factor);
		Node2 = _FactorLink(DownNode, -factor);
		_FactorLink(ModifyNode, -factor) = Node1;
		_FactorLink(DownNode, factor) = ModifyNode;
		_FactorLink(BalanceNode, factor) = Node2;
		_FactorLink(DownNode, -factor) = BalanceNode;
		if (Node1) Node1->Parent = ModifyNode;
		if (Node2) Node2->Parent = BalanceNode;
		DownNode->Parent = BalanceNode->Parent;
		BalanceNode->Parent = DownNode;
		ModifyNode->Parent = DownNode;
		//Set factor according to the result.
		if (DownNode->factor==factor)
		{
			BalanceNode->factor=-factor;
			ModifyNode->factor=0;
		}
		else if (DownNode->factor==0)
		{
			BalanceNode->factor=ModifyNode->factor=0;
		}
		else
		{
			BalanceNode->factor=0;
			ModifyNode->factor=factor;
		}
		DownNode->factor=0;
		if (factor==1) {ModifyNode->rank-=DownNode->rank;DownNode->rank+=BalanceNode->rank;}
		else {DownNode->rank+=ModifyNode->rank;BalanceNode->rank-=DownNode->rank;}
		return DownNode;
	}
	inline TreeNode<T*,D>*&  _FactorLink(TreeNode<T*,D>* Node, __int64 factor)
		//Private helper method to retrieve child according to factor.
		//Return right child if factor>0 and left child otherwise.
	{
		return factor>0? Node->Right : Node->Left;
	}
	void _IncreaseHeight()
	{
		size_t k=(size_t)head.Right;
		head.Right=(TreeNode<T*,D>*)++k;
	}
	void _DecreaseHeight()
	{
		size_t k=(size_t)head.Right;
		head.Right=(TreeNode<T*,D>*)--k;
	}
	TreeNode<T*,D> head;
	fComp fCmp;
	fCopy fCpy;
	fLength fLen;
};

class SCMP
{
public:
	size_t operator()(char* s1,char* s2)
	{
		size_t t=_stricmp(s1,s2);
		if (t==0) return 0;
		return t>0? 1:-1;
	}
};
class SCPY
{
public:
	char* operator()(char* dest, char* src)
	{
		return strcpy(dest,src);
	}
};
class SLEN
{
public:
	size_t operator()(char* str)
	{
		return strlen(str);
	}
};

class WCMP
{
public:
	size_t operator()(wchar_t* s1,wchar_t* s2)
	{
		size_t t=_wcsicmp(s1,s2);
		if (t==0) return 0;
		return t>0? 1:-1;
	}
};
class WCPY
{
public:
	wchar_t* operator()(wchar_t* dest, wchar_t* src)
	{
		return wcscpy(dest,src);
	}
};
class WLEN
{
public:
	size_t operator()(wchar_t* str)
	{
		return wcslen(str);
	}
};