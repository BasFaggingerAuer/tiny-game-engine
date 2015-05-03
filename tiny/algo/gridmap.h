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
#include <limits>

#include <tiny/algo/typecluster.h>

namespace tiny
{

namespace algo
{
	/** A helper function that safely converts floats to ints without using the floor() function to do so. Note that for very large integers this may begin to fail
	  * since the float used by vec3 cannot store it - it's reliable up to a few million. For the GridMap that should easily suffice. */
	inline int convertFloatToInt(float f)
	{
		return (f < 0.0f ? int(f*(1.0f+std::numeric_limits<float>::epsilon()))-1 : int(f));
	}

	/** A helper class to define points on a grid (using signed integers to represent them) and compare these as keys for an std::map. 
	  * For guaranteed safe use, the calculation of the ivec2 location should always be done through use of convertFloatToInt performed on
	  * a location vector component divided by the gridsize. */
	class GridPoint
	{
		private:
			const ivec2 location;
		public:
			GridPoint(const ivec2 & v) : location(v) {}
			GridPoint(const vec3 &v, const float gridsize) : location( ivec2( convertFloatToInt(v.x/gridsize), convertFloatToInt(v.z/gridsize)) ) {}
			const ivec2 & getLocation(void) const { return location; }
			bool operator< (const GridPoint &gp) const { return (location.y == gp.location.y ? location.x < gp.location.x : location.y < gp.location.y); }
/*			bool operator< (const GridPoint &gp) const
			{
				std::cout << "("<<location.x<<","<<location.y<<")<?("<<gp.location.x<<","<<gp.location.y<<"):";
				if(location.y < gp.location.y)
				{
					std::cout << "<y";
				}
				else
				{
					if(location.x < gp.location.x)
					{
						std::cout << "<x";
					}
					else std::cout <<">xy";
				}
				return (location.y < gp.location.y ? true : (location.x < gp.location.x ? true : false) ); 
			}*/
			bool operator== (const GridPoint &gp) const { return (location.x == gp.location.x && location.y == gp.location.y); }
			bool operator== (GridPoint &gp) const { return (location.x == gp.location.x && location.y == gp.location.y); }
	};

	/** An operator overload to allow printing of GridPoint objects. */
	inline std::ostream & operator<< (std::ostream & s, const GridPoint & gp) { s << "(" << gp.getLocation().x << "," << gp.getLocation().y << ")"; return s; }

	template <class T> class GridMap;

	/** The GridMap manages tiles (of a fixed size) in a 2-dimensional grid and provides convenient methods for looking up tiles, creating new tiles and deleting obsolete tiles.
	  * Tiles use an std::map for logarithmic-complexity access of a tile given its location.
	  */
	template <class T> class GridTile : private TypeClusterObject<GridPoint,T>
	{
		private:
		public:
			/** The constructor must set up the TypeClusterObject properly, using its constructor. For this it needs the pointer of the derived class and
			  * it must cast the GridMap (or a class derived from it) back to the TypeCluster class. */
			GridTile(const vec3 & _origin, T * _derivedObject, GridMap<T> * _map) :
				TypeClusterObject<GridPoint,T>(GridPoint(_origin, _map->edgeSize()), _derivedObject, *(static_cast<TypeCluster<GridPoint,T>*>(_map)))
			{
			}

			/** The destructor is nonvirtual since one should not delete the GridTile class directly (like the TypeClusterObject which is never deleted directly).
			  * Whenever the GridMap deletes GridTile objects for cleaning up, it should always do that by calling 'delete' on the derived object. */
			~GridTile(void)
			{
			}
	};

	/** The GridMap clusters TileMapObjects. Its interface is constructed similar to that of the preceding TileCluster class. */
	template <class T> class GridMap : private TypeCluster<GridPoint,T>
	{
		private:
			friend class GridTile<T>; // for the GridTile constructor's cast of GridMap to a TypeCluster.
			double edgesize;
		public:
			/** GridMap constructor. Use farthest possible location as error code (corresponding to the farthest possible tile of the lower left quadrant). */
			GridMap(double _edgesize, std::string name) : TypeCluster<GridPoint,T>(GridPoint(ivec2(std::numeric_limits<int>::min(),std::numeric_limits<int>::min())),name), edgesize(_edgesize) {}

			double edgeSize(void) const { return edgesize; }
			unsigned int numTiles(void) const { return TypeCluster<GridPoint,T>::size(); } /**< Redirects to TypeCluster::size(). */

			/** Get a tile if it exists. If the tile doesn't exist a NULL pointer is returned. */
			T * getTile(vec3 pos) {	return TypeCluster<GridPoint,T>::find(GridPoint(pos,edgesize)); } // Convert vec3 to GridPoint. Any vec3 in the tile should normally be converted to the same GridPoint as the tile's origin itself.

			/** Check for existence of a tile. */
			bool hasTile(vec3 pos) { return (getTile(pos) != 0); }

			/** Get a tile if it exists, and create one if none exist. This function should never return a null pointer. */
			T * safeGetTile(vec3 pos)
			{
				T * tile = getTile(pos);
				if(!tile) tile = new T(pos, this); // Create new tile object if none exists. Simply passing 'pos' is fine since non-origin positions eventually get converted to GridPoint's anyways.
				return tile;
			}

			/** Delete the tile containing 'pos', if it exists. */
			void deleteTile(vec3 pos)
			{
				T * tile = getTile(pos);
				if(tile) delete tile;
			}

			/** Get all tiles closer than 'range' to the position 'pos'. */
			void getMultipleTiles(std::deque<T*> &tilelist, vec3 pos, float range)
			{
				for(unsigned int z = convertFloatToInt(pos.z - range); z <= convertFloatToInt(pos.z + range); z++)
				{
					// For the inner loop cover the full square [x-r,x+r] x [z-r,z+r].
					// It would be better to do the circle (e.g. run x from sqrt(r^2-y^2) for pos=(0,0)) but the gain is minimal and the code sloppier.
					for(unsigned int x = convertFloatToInt(pos.x - range); x <= convertFloatToInt(pos.x + range); x++)
					{
						T * tile = getTile(vec3(x,0,z));
						if(tile) tilelist.push_back(tile);
					}
				}
			}

			// A list of 'using' declarations to expose base class functions publicly.
			using TypeCluster<GridPoint,T>::getName;
			using TypeCluster<GridPoint,T>::is_empty;
			using TypeCluster<GridPoint,T>::size;
			using TypeCluster<GridPoint,T>::begin;
			using TypeCluster<GridPoint,T>::end;
			using TypeCluster<GridPoint,T>::lower_bound;
			using TypeCluster<GridPoint,T>::upper_bound;
	};
} // namespace algo

} // namespace tiny
