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
#include <exception>
#include <string>
#include <vector>

#include <cassert>

#include <tiny/draw/staticmeshhorde.h>

#include <tiny/algo/gridmap.h>

namespace tiny
{

namespace draw
{
	/** A level-of-detail for a specific mesh. MeshType should be a Horde such as WorldIconHorde, StaticMeshHorde or AnimatedMeshHorde. */
	template <typename MeshType, typename HordeInstance>
	class LodMeshHorde
	{
		private:
			std::vector<MeshType*> hordeMeshes; /**< One or more meshes that together form a full Instance. */
			std::vector<HordeInstance> instvec; /**< All instances to be rendered using the hordeMesh of this object. */
		public:
			LodMeshHorde(std::vector<MeshType *> _meshes) : hordeMeshes(_meshes) {}
			~LodMeshHorde(void) {}

			void freeMeshes(void)
			{
				for(unsigned int i = 0; i < hordeMeshes.size(); i++) delete hordeMeshes[i];
			}

			void resetInstances(void) { instvec.clear(); }

			template <typename Iterator>
			void addInstances(Iterator first, Iterator last) { instvec.insert(instvec.end(), first, last); }

			void setInstances(void)
			{
				for(unsigned int i = 0; i < hordeMeshes.size(); i++) hordeMeshes[i]->setInstances(instvec.begin(), instvec.end());
			}
	};

	/** A tile on which instances of a mesh may be placed. The HordeInstance should be WorldIconInstance, StaticMeshInstance or AnimatedMeshInstance. */
	template <typename HordeInstance>
	class HordeTile : public algo::GridTile<HordeTile<HordeInstance> >
	{
		private:
			std::vector<HordeInstance> instances;
			bool active; /**< Tiles can be deactivated if a different, higher quality kind of Instance is active on the same tile. */
		public:
			HordeTile(const vec3 & _origin, algo::GridMap<HordeTile<HordeInstance> > * _map, const std::vector<HordeInstance> & _instvec) :
				algo::GridTile<HordeTile<HordeInstance> >(_origin, this, _map), instances(_instvec), active(true) {}
			~HordeTile(void) { instances.clear(); }

			void setActive(bool _active) { active = _active; }
			bool isActive(void) const { return active; }

			typename std::vector<HordeInstance>::const_iterator first(void) const { return instances.begin(); }
			typename std::vector<HordeInstance>::const_iterator last(void) const { return instances.end(); }
	};

	/** A tile-based Horde that enables native LOD management as well as dynamic memory management (only positions of nearby objects are tracked),
	  * neither of which is present in the default StaticMeshHorde or AnimatedMeshHorde. This is primarily intended for meshes with a high instance density. */
	template <typename MeshType, typename HordeInstance>
	class TiledMeshHorde : public algo::GridMap<HordeTile<HordeInstance> >
	{
		private:
			std::map<float, LodMeshHorde<MeshType,HordeInstance> > lodHordes; /**< A range-keyed LOD map with the highest detail at the start. */
			std::deque<vec3> newTiles; /**< (positions inside) tiles waiting to be initialized. */

			using algo::GridMap<HordeTile<HordeInstance> >::lower_bound;
			using algo::GridMap<HordeTile<HordeInstance> >::upper_bound;
			using algo::GridMap<HordeTile<HordeInstance> >::getName;
		public:
			using algo::GridMap<HordeTile<HordeInstance> >::hasTile;
			using algo::GridMap<HordeTile<HordeInstance> >::edgeSize;
			using algo::GridMap<HordeTile<HordeInstance> >::begin;
			using algo::GridMap<HordeTile<HordeInstance> >::end;

			TiledMeshHorde(double _edgesize, std::string _name) : algo::GridMap<HordeTile<HordeInstance> >(_edgesize, _name) {}
			~TiledMeshHorde(void)
			{
				for(typename std::map<float, LodMeshHorde<MeshType,HordeInstance> >::iterator it = lodHordes.begin(); it != lodHordes.end(); it++) it->second.freeMeshes();
				lodHordes.clear();
			}

			void addLOD(std::vector<MeshType *> _meshes, float _maxRange)
			{
				lodHordes.insert( std::pair<float, LodMeshHorde<MeshType,HordeInstance> >(_maxRange, LodMeshHorde<MeshType,HordeInstance>(_meshes) ) );
			}

			bool getNewTile(vec3 & tilepos)
			{
				if(newTiles.empty()) return false;
				else
				{
					tilepos = newTiles.front();
					newTiles.pop_front();
					return true;
				}
			}

			void addTile(vec3 _location, const std::vector<HordeInstance> & _instvec)
			{
				if(!hasTile(_location)) new HordeTile<HordeInstance>(_location, this, _instvec);
				else std::cerr << " TiledMeshHorde::addTile() : Tile already exists! "<<std::endl;
			}

			void listNewTiles(vec3 center)
			{
				if(lodHordes.size() == 0) return;
				newTiles.clear();
				center.y = 0.0f; // ignore vertical distance
				if(length(center) > 100000*edgeSize() || edgeSize() < 0.1f) { std::cerr << getName() << " : Very small relative edge size, skipping adding tiles. "<<std::endl; return; }
				float range = lodHordes.rbegin()->first;
				for(float y = center.z - range; y < center.z + range; y += edgeSize()*(1.0f+std::numeric_limits<float>::epsilon()))
					for(float x = center.x - range; x < center.x + range; x += edgeSize()*(1.0f+std::numeric_limits<float>::epsilon()))
						if(length(center - vec3(x,0,y)) < range && !hasTile(vec3(x,0,y))) newTiles.push_back(vec3(x,0,y));
				for(unsigned int i = 0; i < newTiles.size(); i++)
				{
					algo::GridPoint gp(newTiles[i],edgeSize());
					newTiles[i] = vec3((gp.getLocation().x+0.0001)*edgeSize(),0.0f,(gp.getLocation().y+0.0001)*edgeSize()); // set newTiles coordinates to the origins of tiles to be created.
				}
			}

			void removeOldTiles(vec3 center)
			{
				center.y = 0.0f; // ignore vertical distance
				std::deque<HordeTile<HordeInstance>*> oldTiles;
				for(typename std::map<algo::GridPoint,HordeTile<HordeInstance>*>::iterator it = begin(); it != end(); it++)
				{
					ivec2 intloc = it->first.getLocation();
					vec3 tilecenter( (intloc.x+0.5)*edgeSize(), 0.0f, (intloc.y+0.5)*edgeSize() );
					if( length(center - tilecenter) > lodHordes.rbegin()->first + 1.5*edgeSize() ) oldTiles.push_back(it->second); // min safety margin against flicker is sqrt(2)*edgesize.
				}
				for(unsigned int i = 0; i < oldTiles.size(); i++) delete oldTiles[i];
				oldTiles.clear();
			}

			/** Recalculate what tile should use what LOD, and create arrays of instances for each LOD. */
			void recalculateLOD(vec3 center)
			{
				center.y = 0.0f; // ignore vertical distance
				if(lodHordes.empty()) return;
				for(typename std::map<float,LodMeshHorde<MeshType,HordeInstance> >::iterator it = lodHordes.begin(); it != lodHordes.end(); it++) it->second.resetInstances();
				for(typename std::map<algo::GridPoint,HordeTile<HordeInstance>*>::iterator it = begin(); it != end(); it++)
				{
					ivec2 intloc = it->first.getLocation();
					vec3 tilecenter( (intloc.x+0.5)*edgeSize(), 0.0f, (intloc.y+0.5)*edgeSize() );
					if( length(center - tilecenter) < lodHordes.rbegin()->first && it->second->isActive())
						lodHordes.upper_bound( length(center-tilecenter) )->second.addInstances(it->second->first(), it->second->last());
					else it->second->setActive(false);
				}
				for(typename std::map<float,LodMeshHorde<MeshType,HordeInstance> >::iterator it = lodHordes.begin(); it != lodHordes.end(); it++) it->second.setInstances();
			}
	};

	/** A tiled horde using tiles with Icons and/or Meshes. */
	class TiledHorde
	{
		private:
			TiledMeshHorde<StaticMeshHorde, StaticMeshInstance> meshHorde;
			TiledMeshHorde<WorldIconHorde, WorldIconInstance> iconHorde;
		public:
			TiledHorde(double _edgesize, std::string _name = "") :
				meshHorde(_edgesize, (_name == "" ? "TiledHorde-mesh" : _name + "-mesh")),
				iconHorde(_edgesize, (_name == "" ? "TiledHorde-icon" : _name + "-icon"))
			{
			}
			~TiledHorde(void) {}

			void removeOldTiles(vec3 center) { meshHorde.removeOldTiles(center); iconHorde.removeOldTiles(center); }
			void listNewTiles(vec3 center) { meshHorde.listNewTiles(center); iconHorde.listNewTiles(center); }
			void recalculateLOD(vec3 center)
			{
				for(std::map<algo::GridPoint,HordeTile<StaticMeshInstance>*>::iterator it = meshHorde.begin(); it != meshHorde.end(); it++) it->second->setActive(true);
				meshHorde.recalculateLOD(center); // this sets Active to false if it falls outside the max LOD range
				// Deactivate all Icon tiles for which a Mesh tile also exists.
				for(std::map<algo::GridPoint,HordeTile<WorldIconInstance>*>::iterator it = iconHorde.begin(); it != iconHorde.end(); it++)
				{
					vec3 loc( (it->first.getLocation().x+0.0001)*iconHorde.edgeSize(), 0.0f, (it->first.getLocation().y+0.0001)*iconHorde.edgeSize());
					it->second->setActive( !meshHorde.hasTile( loc ) || !meshHorde.getTile(loc)->isActive() );
				}
				// Calculate LODs.
				iconHorde.recalculateLOD(center);
			}

			void addLOD(std::vector<StaticMeshHorde *> _meshes, float _maxRange) { meshHorde.addLOD(_meshes, _maxRange); }
			void addLOD(std::vector<WorldIconHorde *> _icons, float _maxRange) { iconHorde.addLOD(_icons, _maxRange); }

			bool getNewStaticMeshTile(vec3 & _pos) { return meshHorde.getNewTile(_pos); }
			bool getNewIconTile(vec3 & _pos) { return iconHorde.getNewTile(_pos); }

			void addTile(vec3 _location, const std::vector<StaticMeshInstance> & _instvec) { meshHorde.addTile(_location, _instvec); }
			void addTile(vec3 _location, const std::vector<WorldIconInstance> & _instvec) { iconHorde.addTile(_location, _instvec); }
	};
} // end namespace tiny

} // end namespace draw
