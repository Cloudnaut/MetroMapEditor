#ifndef ROUTER
#define ROUTER
#include <SFML\System.hpp>
#include <SFML\Window.hpp>
#include <SFML\Graphics.hpp>
#include <list>


namespace Routing
{
	namespace DepthFirstSearch
	{
		class RoutingPoint;

		class Route
		{
		private:
			float m_Distance;
			std::list<RoutingPoint *> m_Path;
		public:
			float GetDistance();
			std::list<RoutingPoint *> *GetPath();
			void AddPoint(RoutingPoint *ToAdd);
		};

		class RoutingPoint
		{
		private:
			sf::Vector2f m_Position;
			std::list<RoutingPoint *> m_Neighbors;
		public:
			sf::Vector2f GetPosition();
			float GetDistanceTo(RoutingPoint &Other);
			bool GetShortestRoute(RoutingPoint *Destination, Route *Path, unsigned int DepthLimit);
			bool GetShortestRoute(RoutingPoint *Destination, Route *Path);
			void Connect(RoutingPoint *ToConnect);
			RoutingPoint(sf::Vector2f Position);
		};
	}

	namespace Dijkstra
	{
		class RoutingPoint;

		class Route
		{
		private:
			float m_Distance;
			std::list<RoutingPoint *> m_Path;
		public:
			float GetDistance();
			std::list<RoutingPoint *> *GetPath();
			void AddPoint(RoutingPoint *ToAdd);
		};

		class DijkstraCalculator
		{
		public:
			void ResetRoutes();
			std::list<RoutingPoint *> m_RoutingPoints;
			RoutingPoint *m_CurrentPoint;
		public:
			void CalculateRoutesFrom(RoutingPoint *Location);
			bool GetShortestRouteTo(RoutingPoint *Destination, Route *ShortestRoute);
		};

		class RoutingPoint
		{
		private:
			sf::Vector2f m_Position;
			std::list<RoutingPoint *> m_Neighbors;
			bool m_Visted;
			float m_Value;
			RoutingPoint *m_Parent;
		public:
			sf::Vector2f GetPosition();
			std::list<RoutingPoint *> *GetNeighbors();
			void Connect(RoutingPoint *ToConnect);
			
			bool IsVisited();
			void SetVisited(bool Value);

			float GetValue();
			void SetValue(float Value);

			RoutingPoint *GetParent();
			void SetParent(RoutingPoint *Parent);

			float GetDistanceTo(RoutingPoint &Other);
			RoutingPoint(sf::Vector2f Position);
		};
	}
}

#endif //ROUTER