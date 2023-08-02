/*-
 * Copyright (c) 2013 Cosku Acay, http://www.coskuacay.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */


/*-
 * A template class that implements a simple stack structure.
 * This demostrates how to use alloctor_traits (specifically with MemoryPool)
 */

#ifndef STACK_ALLOC_H
#define STACK_ALLOC_H

#include <memory>

template <typename T>
struct StackNode_
{
  T data;
  StackNode_* prev;
};

/*
T=int, Alloc=MemoryPool<int>
typedef StackNode_<int> Node;
typedef MemoryPool<int>::rebind<StackNode_<int>>::other allocator
-->
typedef MemoryPool<int>::rebind<StackNode_<int>>::MemoryPool<StackNode_<int>> allocator
-->
typedef MemoryPool<StackNode_<int>> allocator
*/

/** T is the object to store in the stack, Alloc is the allocator to use */
template <class T, class Alloc = std::allocator<T> >
class StackAlloc
{ /*元函数 rebind 用于将一个类型的分配器转换为另一个类型的分配器。其用法是 std::allocator<T>::rebind<U>::other，其中 T 是原始类型，U 是要转换为的目标类型。*/
  public:
    typedef StackNode_<T> Node;
    typedef typename Alloc::template rebind<Node>::other allocator;/*std::allocator<T>::rebind<StackNode_<T>>::other*/

    /** Default constructor */
    StackAlloc() {head_ = 0; }
    /** Default destructor */
    ~StackAlloc() { clear(); }

    /** Returns true if the stack is empty */
    bool empty() {return (head_ == 0);}

    /** Deallocate all elements and empty the stack */
    void clear() {
      Node* curr = head_;
      while (curr != 0)
      {
        Node* tmp = curr->prev;
        allocator_.destroy(curr);
        allocator_.deallocate(curr, 1);
        curr = tmp;
      }
      head_ = 0;
    }

    /** Put an element on the top of the stack */
    void push(T element) {
      Node* newNode = allocator_.allocate(1);/*给内存地址*/
      allocator_.construct(newNode, Node()); /*在内存地址上构造对象*/
      newNode->data = element;/*入栈3步曲:1.给节点数据*/
      newNode->prev = head_;  /*入栈3步曲:2.给节点链接前一个节点*/
      head_ = newNode;        /*入栈3步曲:3.更新当前栈顶节点*/
    }

    /** Remove and return the topmost element on the stack */
    T pop() {
      T result = head_->data;/*出栈3步曲:1.拿出数据*/
      Node* tmp = head_->prev;/*出栈3步曲:2.拿出前一个节点*/
      allocator_.destroy(head_);/*调用节点的析构函数*/
      allocator_.deallocate(head_, 1);/*记录释放的内存*/
      head_ = tmp;/*出栈3步曲:3.更新当前节点*/
      return result;
    }

    /** Return the topmost element */
    T top() { return (head_->data); }

  private:
    allocator allocator_;
    Node* head_;
};

#endif // STACK_ALLOC_H
