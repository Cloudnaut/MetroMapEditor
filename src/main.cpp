#include <SFML\System.hpp>
#include <SFML\Window.hpp>
#include <SFML\Graphics.hpp>
#include <list>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include "Basics.hpp"
#include "Routing.hpp"
using namespace std;

class PathConnection
{
private:
	unsigned int m_StartID;
	unsigned int m_EndID;
	float m_Thickness;
	unsigned int m_Layer;
public:
	unsigned int GetStartID()
	{
		return m_StartID;
	}

	unsigned int GetEndID()
	{
		return m_EndID;
	}

	float GetThickness()
	{
		return m_Thickness;
	}

	unsigned int GetLayer()
	{
		return m_Layer;
	}

	PathConnection(unsigned int StartID, unsigned int EndID, float Thickness, unsigned int Layer)
	{
		m_StartID = StartID;
		m_EndID = EndID;
		m_Thickness = Thickness;
		m_Layer = Layer;
	}
};

class PathPoint : public Routing::Dijkstra::RoutingPoint
{
private:
	unsigned int m_ID;
	sf::Vector2f m_Position;
public:
	void SetID(unsigned int ID)
	{
		m_ID = ID;
	}

	void SetPosition(sf::Vector2f Position)
	{
		m_Position = Position;
	}

	unsigned int GetID()
	{
		return m_ID;
	}

	sf::Vector2f GetPosition()
	{
		return m_Position;
	}

	PathPoint(float X, float Y, unsigned int ID) : Routing::Dijkstra::RoutingPoint(sf::Vector2f(X, Y))
	{
		m_Position = sf::Vector2f(X, Y);
		m_ID = ID;
	}
};

class PathPointManager
{
private:
	unsigned int m_Counter;
	unsigned int m_ConnectionLayerCounter;
public:
	void SetupPool(std::list<PathPoint *> &Points)
	{
		unsigned int Highest = 0;

		for(std::list<PathPoint *>::iterator i = Points.begin(); i != Points.end(); i++)
		{
			if((*i)->GetID() > Highest)
				Highest = (*i)->GetID();
		}

		m_Counter = Highest;
	}

	void SetupConnectionLayerPool(std::list<PathConnection *> &Connections)
	{
		unsigned int Highest = 0;

		for(std::list<PathConnection *>::iterator i = Connections.begin(); i != Connections.end(); i++)
		{
			if((*i)->GetLayer() > Highest)
				Highest = (*i)->GetLayer();
		}

		m_ConnectionLayerCounter = Highest;
	}

	unsigned int RequestID()
	{
		m_Counter++;
		return m_Counter;
	}

	unsigned int RequestConnectionLayer()
	{
		m_ConnectionLayerCounter++;
		return m_ConnectionLayerCounter;
	}

	PathPointManager()
	{
		m_Counter = 0;
		m_ConnectionLayerCounter = 0;
	}
};

class Station : public PathPoint
{
private:
	std::string m_Name;
public:
	std::string GetName()
	{
		return m_Name;
	}

	Station(float X, float Y, unsigned int ID, std::string Name) : PathPoint(X, Y, ID)
	{
		m_Name = Name;
	}
};

bool IsClicked(sf::Clock &Clock, sf::Mouse::Button Button = sf::Mouse::Left)
{
	if(sf::Mouse::isButtonPressed(Button))
	{
		if(Clock.getElapsedTime().asMilliseconds() > 250)
		{
			Clock.restart();
			return true;
		}
	}
	return false;
}

float GetDistance(sf::Vector2f Pos1, sf::Vector2f Pos2)
{
	sf::Vector2f Distance = Pos1 - Pos2;
	return sqrt(pow(Distance.x, 2) + pow(Distance.y, 2));
}

void DrawLine(sf::Vector2f From, sf::Vector2f To, sf::Color Color, float Thickness, sf::RenderWindow *Window)
{
	/*sf::VertexArray Line(sf::Lines, 2);
	Line[0] = sf::Vertex(From, Color);
	Line[1] = sf::Vertex(To, Color);
	Window->draw(Line);*/

	sf::RectangleShape Tunnel;
	Tunnel.setSize(sf::Vector2f(GetDistance(From, To), Thickness));
	Tunnel.setOrigin(0, Tunnel.getSize().y / 2);
	Tunnel.setFillColor(Color);
	Tunnel.setPosition(From);
	
	const float PI = 3.14159265;

    float dx = To.x - From.x;
    float dy = To.y - From.y;

    float rotation = (atan2(dy, dx)) * 180 / PI;

    Tunnel.setRotation(rotation);

	
	Window->draw(Tunnel);
	
}

void DrawPath(list<Routing::Dijkstra::RoutingPoint *> Path, sf::RenderWindow &Target)
{
	Routing::Dijkstra::RoutingPoint *StartPoint = Path.front();

	for(list<Routing::Dijkstra::RoutingPoint *>::iterator i = Path.begin(); i != Path.end();)
	{
		i++;

		if(i == Path.end())
			break;

		DrawLine(StartPoint->GetPosition(), (*i)->GetPosition(), sf::Color::Red, 2, &Target);
		StartPoint = *i;
	}
}

void SaveToFile(std::string FilePath, std::list<PathPoint *> PathPoints, std::list<Station *> Stations, std::list<PathConnection *> PathConnections)
{
	for(std::list<PathPoint *>::iterator CurrentPoint = PathPoints.begin(); CurrentPoint != PathPoints.end(); CurrentPoint++)
	{
		for(std::list<Station *>::iterator CurrentStation = Stations.begin(); CurrentStation != Stations.end(); CurrentStation++)
		{
			if(*CurrentPoint == *CurrentStation)
			{
				CurrentPoint = PathPoints.erase(CurrentPoint);
			}
		}
	}

	std::ofstream Outputfile(FilePath);
	Outputfile << "Metro Map" << endl;

	Outputfile << "[PathPoints]" << std::endl;

	for(std::list<PathPoint *>::iterator i = PathPoints.begin(); i != PathPoints.end(); i++)
	{
		Outputfile << (*i)->GetID() << ':' << (*i)->GetPosition().x << ':' << (*i)->GetPosition().y << std::endl;
	}

	Outputfile << "[Stations]" << std::endl;

	for(std::list<Station *>::iterator i = Stations.begin(); i != Stations.end(); i++)
	{
		Outputfile << (*i)->GetID() << ':' << (*i)->GetPosition().x << ':' << (*i)->GetPosition().y << ':' << (*i)->GetName() << endl;
	}

	Outputfile << "[Connections]" << std::endl;

	for(std::list<PathConnection *>::iterator i = PathConnections.begin(); i != PathConnections.end(); i++)
	{
		Outputfile << (*i)->GetStartID() << ':' << (*i)->GetEndID() << ':' << (*i)->GetThickness() << ':' << (*i)->GetLayer() << std::endl;
	}

}

void LoadFromFile(std::string FilePath, std::list<PathPoint *> &PathPoints, std::list<Station *> &Stations, std::list<PathConnection *> &PathConnections)
{
	std::ifstream InputFile(FilePath);

	int Phase = 0;

	while(!InputFile.eof())
	{
		string CurrentLine;
		
		getline(InputFile, CurrentLine);
		
		if(CurrentLine != "")
		{
			if(CurrentLine == "[PathPoints]")
				Phase = 1;
			else if (CurrentLine == "[Stations]")
				Phase = 2;
			else if (CurrentLine == "[Connections]")
				Phase = 3;
			else
			{
				int SubPhase = 0;

				if(Phase == 1) //Auslesen der Points
				{
					string StringID = "";
					string StringX = "";
					string StringY = "";

					for(int i = 0; i < CurrentLine.length(); i++)
					{
						if(CurrentLine.at(i) == ':')
							SubPhase++;
						else if(SubPhase == 0)
							StringID.push_back(CurrentLine.at(i));
						else if(SubPhase == 1)
							StringX.push_back(CurrentLine.at(i));
						else if(SubPhase == 2)
							StringY.push_back(CurrentLine.at(i));
					}

					unsigned int ID = 0;
					float X = 0;
					float Y = 0;
				
					stringstream Stream;

					Stream << StringID;
					Stream >> ID;
					Stream.clear();

					Stream << StringX;
					Stream >> X;
					Stream.clear();

					Stream << StringY;
					Stream >> Y;
					Stream.clear();

					PathPoints.push_back(new PathPoint(X, Y, ID));
				}
				else if(Phase == 2) //Auslesen der Stations
				{
					string StringID;
					string StringX;
					string StringY;
					string StringName;

					for(int i = 0; i < CurrentLine.length(); i++)
					{
						if(CurrentLine.at(i) == ':')
							SubPhase++;
						else if(SubPhase == 0)
							StringID.push_back(CurrentLine.at(i));
						else if(SubPhase == 1)
							StringX.push_back(CurrentLine.at(i));
						else if(SubPhase == 2)
							StringY.push_back(CurrentLine.at(i));
						else if(SubPhase == 3)
							StringName.push_back(CurrentLine.at(i));
					}

					unsigned int ID = 0;
					float X = 0;
					float Y = 0;

					stringstream Stream;

					Stream << StringID;
					Stream >> ID;
					Stream.clear();

					Stream << StringX;
					Stream >> X;
					Stream.clear();

					Stream << StringY;
					Stream >> Y;
					Stream.clear();

					Station *ToAdd = new Station(X, Y, ID, StringName);

					PathPoints.push_back(ToAdd);
					Stations.push_back(ToAdd);
				}
				else if(Phase == 3) //Auslesen der Connections
				{
					string StringStartID = "";
					string StringEndID = "";
					string StringThickness = "";
					string StringLayer = "";

					for(int i = 0; i < CurrentLine.length(); i++)
					{
						if(CurrentLine.at(i) == ':')
							SubPhase++;
						else if(SubPhase == 0)
							StringStartID.push_back(CurrentLine.at(i));
						else if(SubPhase == 1)
							StringEndID.push_back(CurrentLine.at(i));
						else if(SubPhase == 2)
							StringThickness.push_back(CurrentLine.at(i));
						else if(SubPhase == 3)
							StringLayer.push_back(CurrentLine.at(i));
					}

					unsigned int StartID = 0;
					unsigned int EndID = 0;
					float Thickness = 0;
					unsigned int Layer = 0;

					stringstream Stream;

					Stream << StringStartID;
					Stream >> StartID;
					Stream.clear();

					Stream << StringEndID;
					Stream >> EndID;
					Stream.clear();

					Stream << StringThickness;
					Stream >> Thickness;
					Stream.clear();

					Stream << StringLayer;
					Stream >> Layer;
					Stream.clear();

					PathConnections.push_back(new PathConnection(StartID, EndID, Thickness, Layer));
				}
			}
		}
	}
}

class RoutingWrapper
{
private:
public:
	static PathPoint *GetPathpoint(std::list<PathPoint *> &PathPoints, unsigned int ID)
	{
		for(std::list<PathPoint *>::iterator i = PathPoints.begin(); i != PathPoints.end(); i++)
		{
			if((*i)->GetID() == ID)
				return *i;
		}
		return NULL;
	}

	static void ConnectRoutingPoints(std::list<PathPoint *> &PathPoints, std::list<PathConnection *> &Connections)
	{
		for(std::list<PathConnection *>::iterator i = Connections.begin(); i != Connections.end(); i++)
		{
			PathPoint *StartPoint = GetPathpoint(PathPoints, (*i)->GetStartID());
			PathPoint *EndPoint = GetPathpoint(PathPoints, (*i)->GetEndID());

			StartPoint->Connect(EndPoint);
			EndPoint->Connect(StartPoint);
		}
	}
};

int main()
{
	sf::RenderWindow Window(sf::VideoMode(800 * 1.5, 600 * 1.5), "MapEditor");

	sf::Texture MapImg;

	if(!MapImg.loadFromFile(akilib::GetExecDirectory() + "Map.png"))
	{
		exit(0xDEAD);
	}

	sf::Font StdFont;

	if(!StdFont.loadFromFile(akilib::GetExecDirectory() + "StdFont.ttf"))
	{
		exit(0xDEAD);
	}

	sf::Sprite Map(MapImg);

	float Zoom = 1.0;

	float ViewXPos = 0;
	float ViewYPos = 0;
	float PreviewCursorSize = 2;

	sf::Clock ClickCoolDown;
	ClickCoolDown.restart();

	char EditorMode = 'v';

	//Listen die alle Elemente enthalten

	sf::Color PathPointColor = sf::Color(205, 133, 63);
	sf::Color ConnectionColor = sf::Color::Green;
	sf::Color MoveColor = sf::Color::Yellow;
	sf::Color DeleteColor = sf::Color::Red;
	sf::Color StationColor = sf::Color::Blue;

	PathPointManager PointManager;

	std::list<PathPoint *> PathPoints;
	std::list<PathConnection *> Connections;
	std::list<Station *> Stations;

	LoadFromFile(akilib::GetExecDirectory() + "Metro.txt", PathPoints, Stations, Connections);
	PointManager.SetupPool(PathPoints);
	PointManager.SetupConnectionLayerPool(Connections);
	RoutingWrapper::ConnectRoutingPoints(PathPoints, Connections);

	Routing::Dijkstra::Route ShortestRoute;

	Routing::Dijkstra::DijkstraCalculator DijkstraCalc;

	for(std::list<PathPoint *>::iterator i = PathPoints.begin(); i != PathPoints.end(); i++)
	{
		DijkstraCalc.m_RoutingPoints.push_back(*i);
	}

	/////////////////////////////////////


	while(Window.isOpen())
	{
		//Eventpolling //////////////////////////////////////////

		sf::Event Event;
		Window.pollEvent(Event);

		if(Event.type == sf::Event::MouseWheelMoved)
		{
			if(Event.mouseWheel.delta < 0)
			{
				Zoom += 0.05;
				Event.mouseWheel.delta = 0;
			}
			else if(Event.mouseWheel.delta > 0)
			{
				Zoom -= 0.05;
				Event.mouseWheel.delta = 0;
			}
		}
		
		if(Event.type == sf::Event::Closed)
		{
			Window.close();
			SaveToFile(akilib::GetExecDirectory() + "Metro.txt", PathPoints, Stations, Connections);
		}

		//Mode setting

		if(sf::Keyboard::isKeyPressed(sf::Keyboard::V))
			EditorMode = 'v';
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::R))
			EditorMode = 'r';
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::M))
			EditorMode = 'm';
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			EditorMode = 's';
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::C))
			EditorMode = 'c';
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::P))
			EditorMode = 'p';
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			EditorMode = 'd';
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Add))
			//PreviewCursorSize += 0.1;
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
				PreviewCursorSize += 0.025;
			else PreviewCursorSize += 0.1;
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Subtract))
			//PreviewCursorSize -= 0.1;
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
				PreviewCursorSize -= 0.025;
			else PreviewCursorSize -= 0.1;

		//Scrolling

		sf::Vector2i MousePos = sf::Mouse::getPosition(Window);


		if(MousePos.x > 0 && MousePos.x < Window.getSize().x && MousePos.y > 0 && MousePos.y < Window.getSize().y)
		{

			if(MousePos.x < 100)
			{
				ViewXPos -= (100 - MousePos.x) * 1/10;
			}

			if(MousePos.x > Window.getSize().x - 100)
			{
				ViewXPos += (100 - (Window.getSize().x - MousePos.x)) * 1/10;
			}

			if(MousePos.y < 100)
			{
				ViewYPos -= (100 - MousePos.y) * 1/10;
			}

			if(MousePos.y > Window.getSize().y - 100)
			{
				ViewYPos += (100 - (Window.getSize().y - MousePos.y)) * 1/10;
			}

		}

		sf::View MainView(sf::FloatRect(0, 0, 800 * Zoom, 600 * Zoom));
		MainView.setCenter(ViewXPos, ViewYPos);

		////////////////////////////////////////////////////////

		Window.setView(MainView);

		//////////////////////////////////////////////////////////////

		Window.clear();

		Window.draw(Map);

		
		//Editing

		sf::Text EditorModeTxt;
		EditorModeTxt.setFont(StdFont);
		EditorModeTxt.setPosition(ViewXPos - ((800 * Zoom) / 2) + 15 * Zoom, ViewYPos - ((600 * Zoom) / 2) + 15 * Zoom);
		EditorModeTxt.setScale(Zoom * 0.5, Zoom * 0.5);
		EditorModeTxt.setColor(sf::Color::Red);
		EditorModeTxt.setString("Editor Mode: ");
		
		bool PreviewVisible = true;
		sf::CircleShape PreviewCursor;
		PreviewCursor.setPosition(Window.mapPixelToCoords(sf::Mouse::getPosition(Window)));
		PreviewCursor.setRadius(PreviewCursorSize);
		PreviewCursor.setOrigin(PreviewCursor.getRadius(), PreviewCursor.getRadius());

		if(EditorMode == 'v') //View Mode
		{
			EditorModeTxt.setString(EditorModeTxt.getString() + "View");
			PreviewVisible = false;
		}
		else if(EditorMode == 'r') //Route Mode
		{
			EditorModeTxt.setString(EditorModeTxt.getString() + "Routing");

			PreviewCursor.setFillColor(DeleteColor);

			static bool StartSelected = false;
			static bool Calculated = false;
			static PathPoint *StartPoint = NULL;
			static PathPoint *EndPoint = NULL;

			if(StartPoint == NULL || EndPoint == NULL)
			{
				if(!StartSelected)
				{
					if(IsClicked(ClickCoolDown))
					{
						for(std::list<PathPoint *>::iterator i = PathPoints.begin(); i != PathPoints.end(); i++)
						{
							if(GetDistance((*i)->GetPosition(), PreviewCursor.getPosition()) < 2 )
							{
								StartPoint = *i;
								StartSelected = true;
								break;
							}
						}
					}
				}
				else
				{
					sf::CircleShape Focus;
					Focus.setFillColor(sf::Color::Transparent);
					Focus.setOutlineColor(DeleteColor);
					Focus.setOutlineThickness(2);
					Focus.setRadius(15);
					Focus.setOrigin(Focus.getRadius(), Focus.getRadius());
					Focus.setPosition(StartPoint->GetPosition());
					Window.draw(Focus);

					if(IsClicked(ClickCoolDown))
					{
						for(std::list<PathPoint *>::iterator i = PathPoints.begin(); i != PathPoints.end(); i++)
						{
							if(GetDistance((*i)->GetPosition(), PreviewCursor.getPosition()) < 2 )
							{
								EndPoint = *i;
								StartSelected = false;
								break;
							}
						}
					}
				}
			}
			else //Es wurden zwei Punkte ausgewählt
			{
				sf::CircleShape Focus;
				Focus.setFillColor(sf::Color::Transparent);
				Focus.setOutlineColor(DeleteColor);
				Focus.setOutlineThickness(2);
				Focus.setRadius(15);
				Focus.setOrigin(Focus.getRadius(), Focus.getRadius());
				
				Focus.setPosition(StartPoint->GetPosition());
				Window.draw(Focus);
				Focus.setPosition(EndPoint->GetPosition());
				Window.draw(Focus);
				
				if(!Calculated)
				{
					DijkstraCalc.CalculateRoutesFrom(StartPoint);
					
					if(!DijkstraCalc.GetShortestRouteTo(EndPoint, &ShortestRoute))
					{
						exit(0xDEAD);
					}

					Calculated = true;
				}

				string StringDistance;
				stringstream Distance;
				Distance << ShortestRoute.GetDistance();
				Distance >> StringDistance;

				EditorModeTxt.setString(sf::String(StringDistance));
			}

			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			{
				StartPoint = NULL;
				EndPoint = NULL;
				Calculated = false;
				ShortestRoute.GetPath()->clear();
				DijkstraCalc.ResetRoutes();
			}
		}
		else if(EditorMode == 'm') //Move Mode
		{
			EditorModeTxt.setString(EditorModeTxt.getString() + "Move");

			PreviewCursor.setFillColor(MoveColor);

			static bool MoveStarted = false;
			static PathPoint *CurrentPathPoint = NULL;

			if(!MoveStarted)
			{
				if(IsClicked(ClickCoolDown))
				{
					for(std::list<PathPoint *>::iterator i = PathPoints.begin(); i != PathPoints.end(); i++)
					{
						if(GetDistance((*i)->GetPosition(), PreviewCursor.getPosition()) < 2 )
						{
							CurrentPathPoint = *i;
							MoveStarted = true;
							break;
						}
					}
				}
			}
			else
			{
				CurrentPathPoint->SetPosition(PreviewCursor.getPosition());

				if(IsClicked(ClickCoolDown))
					MoveStarted = false;
			}
		}
		else if(EditorMode == 's') //Add Staation Mode
		{
			EditorModeTxt.setString(EditorModeTxt.getString() + "Add Station");

			PreviewCursor.setFillColor(sf::Color::Transparent);
			PreviewCursor.setOutlineColor(StationColor);
			PreviewCursor.setOutlineThickness(2);

			if(IsClicked(ClickCoolDown))
			{
				Station *ToAdd = new Station(PreviewCursor.getPosition().x, PreviewCursor.getPosition().y, PointManager.RequestID(), "DEFAULT");

				Stations.push_back(ToAdd);
				PathPoints.push_back(ToAdd);
			}
		}
		else if(EditorMode == 'c') //Connect MOde
		{
			static bool ConnectionStarted = false;
			static PathPoint *CurrentPathPoint = NULL;

			static float TunnelThickness = 10;
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::F1))
				TunnelThickness = 2;
			else if(sf::Keyboard::isKeyPressed(sf::Keyboard::F2))
				TunnelThickness = 10;

			EditorModeTxt.setString(EditorModeTxt.getString() + "Connect");
			PreviewCursor.setFillColor(ConnectionColor);

			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			{
				ConnectionStarted = false;
			}

			if(!ConnectionStarted)
			{
				for(std::list<PathPoint *>::iterator i = PathPoints.begin(); i != PathPoints.end(); i++)
				{
					if(GetDistance((*i)->GetPosition(), PreviewCursor.getPosition()) < 2 )
					{
						if(IsClicked(ClickCoolDown))
						{
							ConnectionStarted = true;
							CurrentPathPoint = *i;
						}
					}
				}
			}

			if(ConnectionStarted)
			{
				DrawLine(CurrentPathPoint->GetPosition(), PreviewCursor.getPosition(), ConnectionColor, TunnelThickness, &Window);

				if(IsClicked(ClickCoolDown))
				{
					for(std::list<PathPoint *>::iterator i = PathPoints.begin(); i != PathPoints.end(); i++)
					{
						if(GetDistance((*i)->GetPosition(), PreviewCursor.getPosition()) < 2 )
						{
							if(*i != CurrentPathPoint)
							{
								Connections.push_back(new PathConnection(CurrentPathPoint->GetID(), (*i)->GetID(), TunnelThickness, PointManager.RequestConnectionLayer()));
								ConnectionStarted = false;
							}
						}
					}
				}

				if(IsClicked(ClickCoolDown, sf::Mouse::Right))
				{
					PathPoint *NewPathPoint = new PathPoint(PreviewCursor.getPosition().x, PreviewCursor.getPosition().y, PointManager.RequestID());

					PathPoints.push_back(NewPathPoint);

					Connections.push_back(new PathConnection(CurrentPathPoint->GetID(), NewPathPoint->GetID(), TunnelThickness, PointManager.RequestConnectionLayer()));

					CurrentPathPoint = NewPathPoint;
				}
			}
		}
		else if(EditorMode == 'p') //Pathpoint Mode
		{
			EditorModeTxt.setString(EditorModeTxt.getString() + "Add PathPoint");
			PreviewCursor.setFillColor(PathPointColor);
			
			if(IsClicked(ClickCoolDown))
			{
				PathPoints.push_back(new PathPoint(PreviewCursor.getPosition().x, PreviewCursor.getPosition().y, PointManager.RequestID()));
			}
		}
		else if(EditorMode == 'd') //Delete Mode
		{
			EditorModeTxt.setString(EditorModeTxt.getString() + "Delete");

			PreviewCursor.setFillColor(DeleteColor);

			static PathPoint *CurrentPathPoint = NULL;

			if(IsClicked(ClickCoolDown))
			{
				for(std::list<PathPoint *>::iterator PointToDelete = PathPoints.begin(); PointToDelete != PathPoints.end(); PointToDelete++)
				{
					if(GetDistance((*PointToDelete)->GetPosition(), PreviewCursor.getPosition()) < 2 )
					{
						///////// Connections vom zum löschenden Point löschen

						for(std::list<PathConnection *>::iterator i = Connections.begin(); i != Connections.end(); i++)
						{
							if((*i)->GetStartID() == (*PointToDelete)->GetID() || (*i)->GetEndID() == (*PointToDelete)->GetID())
							{
								delete *i;
								Connections.erase(i);
								i = Connections.begin();
							}
						}

						///////// Point selber löschen
						
						(*PointToDelete)->SetID(NULL);
						PathPoints.erase(PointToDelete);
						PointToDelete = PathPoints.begin();
					}
				}
			}
		}

		////////////////////////////////////

		for(std::list<PathConnection *>::iterator CurrentConnection = Connections.begin(); CurrentConnection != Connections.end(); CurrentConnection++)
		{
			PathPoint *BeginPoint = NULL;
			PathPoint *EndPoint = NULL;

			for(std::list<PathPoint *>::iterator CurrentPoint = PathPoints.begin(); CurrentPoint != PathPoints.end(); CurrentPoint++)
			{
				if((*CurrentConnection)->GetStartID() == (*CurrentPoint)->GetID())
				{
					BeginPoint = *CurrentPoint;
				}

				if((*CurrentConnection)->GetEndID() == (*CurrentPoint)->GetID())
				{
					EndPoint = *CurrentPoint;
				}
			}
			
			DrawLine(BeginPoint->GetPosition(), EndPoint->GetPosition(), ConnectionColor, (*CurrentConnection)->GetThickness(), &Window);
		}

		for(std::list<Station *>::iterator i = Stations.begin(); i != Stations.end(); i++) //Nicht mehr nötige Stations löschen
		{
			if((*i)->GetID() == NULL)
			{
				delete *i;
				Stations.erase(i);
				i = Stations.begin();
			}
		}

		for(std::list<Station *>::iterator i = Stations.begin(); i != Stations.end(); i++)
		{
			sf::CircleShape StationShape;
			StationShape.setFillColor(StationColor);
			StationShape.setPosition((*i)->GetPosition());
			StationShape.setRadius(8);
			StationShape.setOrigin(StationShape.getRadius(), StationShape.getRadius());
			Window.draw(StationShape);
		}

		for(std::list<PathPoint *>::iterator i = PathPoints.begin(); i != PathPoints.end(); i++)
		{
			sf::CircleShape PathPointShape;
			PathPointShape.setFillColor(PathPointColor);
			PathPointShape.setPosition((*i)->GetPosition());
			PathPointShape.setRadius(2);
			PathPointShape.setOrigin(PathPointShape.getRadius(), PathPointShape.getRadius());
			Window.draw(PathPointShape);
		}

		if(ShortestRoute.GetPath()->size() > 0)
		{
			/*for(std::list<Routing::RoutingPoint *>::iterator i = ShortestRoute.GetPath()->begin(); i != ShortestRoute.GetPath()->end(); i++)
			{
				sf::CircleShape Mark;
				Mark.setRadius(3);
				Mark.setOrigin(Mark.getRadius(), Mark.getRadius());
				Mark.setPosition((*i)->GetPosition());
				Mark.setFillColor(sf::Color::Red);
				Window.draw(Mark);
			}*/

			DrawPath(*ShortestRoute.GetPath(), Window);
		}

		if(PreviewVisible)
			Window.draw(PreviewCursor);

		Window.draw(EditorModeTxt);

		Window.display();
	}
}