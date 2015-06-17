/*
Copyright 2015, Matthijs van Dorp.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <map>

namespace tiny
{

namespace algo
{
	/**< Forward declaration of TypeClusterObject, which are objects that are clustered by the TypeCluster class. */
	template <class K, class T> class TypeClusterObject;

	/** The Cluster is a container class, which is designed to extend default std::map functionality by ensuring that all objects of a class are
	  * collected and managed together (*). Furthermore, it can temporarily exclude some objects from the functionality by hiding them in another map.
	  *
	  * Objects to be clustered should be derived from the TypeClusterObject class, which takes care of the automatic subscription and unsubscription.
	  * The TypeClusterObject class itself is not inserted into a map, instead the derived class is.
	  *
	  * (*) That is, the TypeClusterObject constructor has the form (K, T*, TypeCluster<K,T>&) which enforces that every object is not only given a key but also
	  * a map which contains the key. The TypeClusterObject takes care of adding the T object to the map. At deletion of the derived object the TypeClusterObject
	  * ensures that the deleted object is erased from the map.
	  */
	template <class K, class T> class TypeCluster
	{
		private:
			template <class R, class S> friend class TypeClusterObject; /**< We will let the TypeClusterObject add and remove itself through private functions.*/

			std::string typeClusterName; /**< The name of the TypeCluster. This way, diagnostic messages can also specify what derived class is causing the problems.*/
			std::map<K,T*> cluster; /**< A container for all T objects that have been created (unless unclustered through UnclusterObject()). */
			std::map<K,T*> excluster; /**< A container for elements that are tagged to be excluded from the cluster, through using UnclusterObject(). */

			/** A key to be assigned to a TypeClusterObject whenever its passed key is invalid for whatever reason. Such ClusterObjects will not become
			  * part of the Cluster and any cluster operations will therefore fail to find it. */
			const K errorKey;

			/** Subscribe a TypeClusterObject to the cluster. The statement tries insertion after checks pass. If insertion fails anyways, insert() returns
			  * map::end() and the function still returns false. A 'true' is only returned if insertion was successful. */
			bool subscribe(const K & key, T * elt)
			{
//				std::cout << " key=" <<key << " errorKey="<<errorKey<<" cluster check:"<<(cluster.find(key)!=cluster.end()?"fail":"pass");
//				std::cout << " full check: "<<((key == errorKey || cluster.find(key) != cluster.end() || excluster.find(key) != excluster.end()) ? "fail" : "pass")<<std::endl;
				return ( (key == errorKey || cluster.find(key) != cluster.end() || excluster.find(key) != excluster.end()) ? false : cluster.insert(std::pair<K,T*>(key, elt)).second);
			}

			/** Unsubscribe (delete) a TypeClusterObject from the cluster. */
			void unsubscribe(const K & key)
			{
				if(!cluster.erase(key) && !excluster.erase(key))
				{
					std::cerr << " TypeCluster::unsubscribe() : Object of class "<<typeClusterName<<" with key "<<key<<" not found! ";
				}
			}
		protected:
		public:
			/** The constructor takes the unique key (first template argument) and a descriptive name (used for error printing) as its arguments. */
			TypeCluster(const K & e, std::string _name) : typeClusterName(_name), errorKey(e)
			{
			}

			/** The destructor. Clean up the TypeCluster, but do it properly: delete the derived classes, so that all the destructors are properly called even though
			  * destructors are not virtual. */
			~TypeCluster(void)
			{
				while(cluster.size() > 0) delete cluster.begin()->second;
			}

			std::string getName(void) const { return typeClusterName; } /**< Get the name of the TypeCluster. */

			/** Allow deleting a member from the cluster without deleting the object itself. */
			void UnclusterObject(K _key)
			{
				typename std::map<K,T*>::iterator _it = cluster.find(_key);
				if(_it == cluster.end())
				{
					std::cerr << " TypeCluster::UnclusterObject() : Cannot uncluster iterator std::map::end()! "<<std::endl;
					return;
				}
				else
				{
					excluster.insert(*_it);
					cluster.erase(_it);
				}
			}

			/** Allow un-deleting a member, re-adding it to the active cluster. Can only be called if UnclusterObject was called before. */
			void ReclusterObject( K key )
			{
				if(excluster.find(key) != excluster.end())
				{
					typename std::map<K,T*>::iterator it = excluster.find(key);
					cluster.insert(std::make_pair(it->first, it->second));
					excluster.erase(it);
				}
				else
				{
					std::cerr << " TypeCluster::ReclusterObject() : Warning: Couldn't find element to be reclustered. Object with key "<< key<<" will still not appear in the cluster."<<std::endl;
				}
			}

			/** A way to find the object keyed by the unique key '_key'. Do NOT store these pointers (for safety reasons), you never know whether erase() was called since you last got the pointer.
			  * Instead, store the key and ask for the T object when it is required. */
			T * find(K key)
			{
				typename std::map<K,T*>::iterator it = cluster.find(key);
				return ( (it == cluster.end()) ? 0 : it->second);
			}

			T * operator[] (const K & key) { return find(key); }

			/** Signals whether the cluster is empty. */
			bool is_empty(void)
			{
				return cluster.empty();
			}

			/** Returns the number of members in the cluster. */
			unsigned int size(void)
			{
				return cluster.size();
			}

			/** The cluster itself is private, but you can linearly access the contents of the cluster by using the ++ operator and using the first/last iterators. */
			typename std::map<K,T*>::iterator begin(void)
			{
				return cluster.begin();
			}

			/** The cluster itself is private, but you can linearly access the contents of the cluster by using the ++ operator and using the first/last iterators. */
			typename std::map<K,T*>::iterator end(void)
			{
				return cluster.end();
			}

			/** Gives the cluster's lower bound using the provided key. Uses std::map::lower_bound(). */
			typename std::map<K,T*>::iterator lower_bound(K & key)
			{
				return cluster.lower_bound(key);
			}

			/** Gives the cluster's upper bound using the provided key. Uses std::map::upper_bound(). */
			typename std::map<K,T*>::iterator upper_bound(K & key)
			{
				return cluster.upper_bound(key);
			}
	};

	/** The base class for objects to be clustered. Deriving from this class should be done in the CRTP way, i.e. class A : public TypeClusterObject<K,A>. */
	template <class K, class T> class TypeClusterObject
	{
		private:
			K key; /**< A unique key that is used to identify the element it is associated to. */
			T * elt; /**< Pointer to the element that is being clustered - essentially 'this' but then the derived class. */
			TypeCluster<K,T> & typeCluster; /**< Reference to the TypeCluster class that controls the TypeClusterObject's. */

			TypeClusterObject(const TypeClusterObject &); /**< NO copy construction - it should not be necessary and unintended copy construction will likely cause fatal errors or memory leakage. */

		protected:
			/** TypeClusterObject destructor. It is protected, because you should never call 'delete' on a TypeClusterObject*, but always on the derived class
			  * (also note that this function is not virtual since you shouldn't destroy it anyways). */
			~TypeClusterObject(void)
			{
				typeCluster.unsubscribe(key); // Delete from TypeCluster.
			}

			/** Constructor. Add ourselves to the TypeCluster. */
			TypeClusterObject(K _key, T * derivedObject, TypeCluster<K,T> & tc) : key(_key), elt(derivedObject), typeCluster(tc)
			{
				if(!typeCluster.subscribe(key,elt))
				{
					std::cerr << " Error: TypeClusterObject() : Key "<<key<<" is not allowed in TypeCluster "<<typeCluster.getName()<<"! Cannot add! "<<std::endl;
				}
			}

			const K & getKey(void) const
			{
				return key;
			}
		public:
	};
} // end namespace algo

} // end namespace tiny
