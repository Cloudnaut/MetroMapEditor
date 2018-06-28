#include "Routing.hpp"

using namespace Routing;


float DepthFirstSearch::Route::GetDistance()
{
	return m_Distance;
}

std::list<DepthFirstSearch::RoutingPoint *> *DepthFirstSearch::Route::GetPath()
{
	return &m_Path;
}

void DepthFirstSearch::Route::AddPoint(RoutingPoint *ToAdd)
{
	if(m_Path.size() > 0)
		m_Distance += m_Path.back()->GetDistanceTo(*ToAdd);
	m_Path.push_back(ToAdd);
}

sf::Vector2f DepthFirstSearch::RoutingPoint::GetPosition()
{
	return m_Position;
}

float DepthFirstSearch::RoutingPoint::GetDistanceTo(RoutingPoint &Other)
{
	sf::Vector2f Distance = GetPosition() - Other.GetPosition();
	return sqrt(pow(Distance.x, 2) + pow(Distance.y, 2));
}

bool DepthFirstSearch::RoutingPoint::GetShortestRoute(RoutingPoint *Destination, Route *Path, unsigned int DepthLimit)
{
	if(DepthLimit != NULL)
	{
		if(Path->GetPath()->size() > DepthLimit)
			return false;
	}

	for(std::list<RoutingPoint *>::iterator i = Path->GetPath()->begin(); i != Path->GetPath()->end(); i++) //Wurde Point schon mal übergangen
	{
		if((*i) == this)
			return false;
	}

	Path->AddPoint(this);

	if(Destination == this)
	{
		return true;
	}
	else
	{
		std::list<Route> *Routes = new std::list<Route>;

		for(std::list<RoutingPoint *>::iterator i = m_Neighbors.begin(); i != m_Neighbors.end(); i++)
		{
			Route *CurrentRoute = new Route;
			*CurrentRoute = *Path;

			if((*i)->GetShortestRoute(Destination, CurrentRoute, DepthLimit))
			{
				Routes->push_back(*CurrentRoute);
			}

			delete CurrentRoute;
		}

		if(Routes->empty())
		{
			delete Routes;
			return false;
		}
		else
		{
			Route ShortestRoute = Routes->front();

			for(std::list<Route>::iterator CurrentRoute = Routes->begin(); CurrentRoute != Routes->end(); CurrentRoute++)
			{
				if((*CurrentRoute).GetDistance() < ShortestRoute.GetDistance())
					ShortestRoute = *CurrentRoute;
			}

			delete Routes;

			*Path = ShortestRoute;
			return true;
		}
	}
}


bool DepthFirstSearch::RoutingPoint::GetShortestRoute(RoutingPoint *Destination, Route *Path)
{
	unsigned int CurrentDepth = 1 + 10;
	bool PathFound = false;
	Route *ShortestRoute = new Route;

	while(!GetShortestRoute(Destination, ShortestRoute, CurrentDepth))
	{
		ShortestRoute->GetPath()->clear();
		CurrentDepth++;
	}

	PathFound = true;
	*Path = *ShortestRoute;
	delete ShortestRoute;
	return true;

}


void DepthFirstSearch::RoutingPoint::Connect(RoutingPoint *ToConnect)
{
	m_Neighbors.push_back(ToConnect);
}

DepthFirstSearch::RoutingPoint::RoutingPoint(sf::Vector2f Position)
{
	m_Position = Position;
}

//Dijktra



float Dijkstra::Route::GetDistance()
{
	return m_Distance;
}

std::list<Dijkstra::RoutingPoint *> *Dijkstra::Route::GetPath()
{
	return &m_Path;
}

void Dijkstra::Route::AddPoint(RoutingPoint *ToAdd)	
{
	m_Path.push_back(ToAdd);
}

void Dijkstra::DijkstraCalculator::ResetRoutes()
{
	for(std::list<RoutingPoint *>::iterator i = m_RoutingPoints.begin(); i != m_RoutingPoints.end(); i++)
	{
		m_CurrentPoint = NULL;
		(*i)->SetVisited(false);
		(*i)->SetValue(NULL);
		(*i)->SetParent(NULL);
	}
}

void Dijkstra::DijkstraCalculator::CalculateRoutesFrom(RoutingPoint *Location)
{
	m_CurrentPoint = Location;
	m_CurrentPoint->SetVisited(true);

	for(std::list<RoutingPoint *>::iterator i = m_CurrentPoint->GetNeighbors()->begin(); i != m_CurrentPoint->GetNeighbors()->end(); i++)
	{
		if(!(*i)->IsVisited())
		{
			float CalculatedValue = m_CurrentPoint->GetValue() + m_CurrentPoint->GetDistanceTo(**i);

			if(CalculatedValue < (*i)->GetValue() || (*i)->GetParent() == NULL)
			{
				(*i)->SetValue(CalculatedValue);
				(*i)->SetParent(m_CurrentPoint);
			}
		}
	}

	RoutingPoint *LowestValue = NULL;

	for(std::list<RoutingPoint *>::iterator i = m_RoutingPoints.begin(); i != m_RoutingPoints.end(); i++)
	{
		if(!(*i)->IsVisited() && (*i)->GetParent() != NULL)
		{
			LowestValue = *i;
			break;
		}
	}

	if(LowestValue == NULL)
	{
		return;
	}

	//Muss aus sicht Aller seien

	for(std::list<RoutingPoint *>::iterator i = m_RoutingPoints.begin(); i != m_RoutingPoints.end(); i++)
	{
		if(!(*i)->IsVisited() && (*i)->GetParent() != NULL)
			if((*i)->GetValue() < LowestValue->GetValue())
				LowestValue = (*i);
	}

	if(LowestValue->IsVisited())
		return;

	CalculateRoutesFrom(LowestValue);
}

bool Dijkstra::DijkstraCalculator::GetShortestRouteTo(RoutingPoint *Destination, Route *ShortestRoute)
{
	ShortestRoute->GetPath()->clear();

	RoutingPoint *CurrentPoint = Destination;

	while(CurrentPoint->GetParent() != NULL)
	{
		ShortestRoute->GetPath()->push_front(CurrentPoint);
		CurrentPoint = CurrentPoint->GetParent();
	}

	ShortestRoute->GetPath()->push_front(CurrentPoint);
}



sf::Vector2f Dijkstra::RoutingPoint::GetPosition()
{
	return m_Position;
}

std::list<Dijkstra::RoutingPoint *> *Dijkstra::RoutingPoint::GetNeighbors()
{
	return &m_Neighbors;
}

void Dijkstra::RoutingPoint::Connect(RoutingPoint *ToConnect)
{
	m_Neighbors.push_back(ToConnect);
}

	
bool Dijkstra::RoutingPoint::IsVisited()
{
	return m_Visted;
}

void Dijkstra::RoutingPoint::SetVisited(bool Value)
{
	m_Visted = Value;
}


float Dijkstra::RoutingPoint::GetValue()
{
	return m_Value;
}

void Dijkstra::RoutingPoint::SetValue(float Value)
{
	m_Value = Value;
}


Dijkstra::RoutingPoint *Dijkstra::RoutingPoint::GetParent()
{
	return m_Parent;
}

void Dijkstra::RoutingPoint::SetParent(RoutingPoint *Parent)
{
	m_Parent = Parent;
}


float Dijkstra::RoutingPoint::GetDistanceTo(RoutingPoint &Other)
{
	sf::Vector2f Distance = GetPosition() - Other.GetPosition();
	return sqrt(pow(Distance.x, 2) + pow(Distance.y, 2));
}

Dijkstra::RoutingPoint::RoutingPoint(sf::Vector2f Position)
{
	m_Position = Position;
	m_Visted = false;
	m_Value = NULL;
	m_Parent = NULL;
}
