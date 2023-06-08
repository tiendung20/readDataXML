#include "pugixml.hpp"
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

typedef std::pair<double, double> Point;
typedef std::tuple<Point, Point, double> Shape;
typedef std::tuple<std::string, Shape> Edge;
typedef std::tuple<std::string, std::vector<Point>> Junc;
typedef std::tuple<std::string, int, Shape> Part;
using namespace std;

ofstream myfile;

vector<std::pair<Point, Point>> listPoints(Point A, Point B, double x)
{
    double d = sqrt(pow(B.first - A.first, 2) + pow(B.second - A.second, 2));
    int n = int(d / x);
    float x1, y1, x2, y2;
    vector<std::pair<Point, Point>> list;
    double ratio = x / d;
    if (n == 0)
    {
        list.push_back({A, B});
        return list;
    }

    for (int i = 0; i < n; i++)
    {
        x1 = A.first + i * (B.first - A.first) * ratio;
        y1 = A.second + i * (B.second - A.second) * ratio;
        x2 = A.first + (i + 1) * (B.first - A.first) * ratio;
        y2 = A.second + (i + 1) * (B.second - A.second) * ratio;
        list.push_back({Point({x1, y1}), Point({x2, y2})});
    }
    if (d / x != int(d / x))
    {
        list.push_back({Point({x2, y2}), B});
    }

    return list;
}

void splitEdge(double x, Edge *e)
{
    vector<std::pair<Point, Point>> list = listPoints(get<0>(get<1>(*e)), get<1>(get<1>(*e)), x);
    int i = 0;
    for (std::pair<Point, Point> point : list)
    {
        myfile << get<0>(*e) << " " << i << "_" << fixed << setprecision(2) << point.first.first << "," << point.first.second << "_" << point.second.first << "," << point.second.second << "\n";
        i++;
    }
}

void splitJunc(double x, Junc *j)
{
    vector<Point> points = get<1>(*j);
    int n = 0;
    for (int i = 0; i < points.size() - 1; i++)
    {
        vector<std::pair<Point, Point>> list = listPoints(points[i], points[i + 1], x);
        for (std::pair<Point, Point> point : list)
        {
            myfile << get<0>(*j) << " " << n << "_" << point.first.first << "," << point.first.second << "_" << point.second.first << "," << point.second.second << "\n";
            n++;
        }
    }
}

vector<Edge> getSucc(vector<Edge> *edges, Edge *e)
{
    vector<Edge> list;
    for (Edge edge : *edges)
    {
        if (get<0>(get<1>(edge)) == get<1>(get<1>(*e)))
        {
            list.push_back(edge);
        }
    }
    return list;
}

int main()
{
    vector<Edge> edges;
    vector<Junc> juncs;
    vector<Edge> starts;
    vector<Edge> ends;
    vector<string> dead_end;
    const char *filename = "vd013.net.xml";
    myfile.open("AllParts.txt");
    pugi::xml_document doc;
    if (!doc.load_file(filename))
    {
        cout << "Can't read" << endl;
        return 0;
    }
    pugi::xml_node panels = doc.child("net");
    pugi::xml_attribute attr;
    for (pugi::xml_node panel = panels.child("junction"); panel; panel = panel.next_sibling("junction"))
    {
        attr = panel.first_attribute();
        if (strcmp(attr.next_attribute().name(), "type") == 0 && strcmp(attr.next_attribute().value(), "dead_end") == 0)
        {
            dead_end.push_back(attr.value());
        }
    }
    for (pugi::xml_node panel = panels.child("edge"); panel; panel = panel.next_sibling("edge"))
    {
        attr = panel.first_attribute();
        if (strcmp(attr.next_attribute().name(), "from") == 0 && strcmp(attr.next_attribute().next_attribute().name(), "to") == 0 && strcmp(attr.next_attribute().value(), attr.next_attribute().next_attribute().value()) != 0)
        {
            int check1 = 0;
            int check2 = 0;
            for (string s : dead_end)
            {
                if (strcmp(attr.next_attribute().value(), s.c_str()) == 0)
                {
                    check1 = 1;
                }
                if (strcmp(attr.next_attribute().next_attribute().value(), s.c_str()) == 0)
                {
                    check2 = 1;
                }
                if (check1 == 1 && check2 == 1)
                {
                    break;
                }
            }
            Edge e;
            get<0>(e) = attr.value();
            string shape;
            double length;
            int i = 0;
            for (pugi::xml_node child = panel.first_child(); child; child = child.next_sibling())
            {
                attr = child.first_attribute().next_attribute().next_attribute();
                if (strcmp(attr.name(), "disallow") == 0 && strcmp(attr.value(), "pedestrian") == 0)
                {
                    shape = attr.next_attribute().next_attribute().next_attribute().value();
                    length = atof(attr.next_attribute().next_attribute().value());
                    i++;
                }
                if (i > 1)
                    break;
            }
            if (i == 1)
            {
                vector<double> nums;
                stringstream ss(shape);
                double num;
                while (ss >> num)
                {
                    nums.push_back(num);
                    if (ss.peek() == ',' || ss.peek() == ' ')
                    {
                        ss.ignore();
                    }
                }
                Shape s = Shape({Point({nums[0], nums[1]}), Point({nums[2], nums[3]}), length});
                get<1>(e) = s;
                if (check1 == 1)
                {
                    starts.push_back(e);
                }
                if (check2 == 1)
                {
                    ends.push_back(e);
                }

                edges.push_back(e);
            }
        }
        else if (strcmp(attr.next_attribute().name(), "function") == 0 && strcmp(attr.next_attribute().value(), "internal") == 0)
        {
            Junc j;
            get<0>(j) = attr.value();
            get<0>(j).erase(0, 1);
            string shape;
            double length;
            int i = 0;
            for (pugi::xml_node child = panel.first_child(); child; child = child.next_sibling())
            {
                attr = child.first_attribute().next_attribute().next_attribute();
                if (strcmp(attr.name(), "disallow") == 0 && strcmp(attr.value(), "pedestrian") == 0)
                {
                    shape = attr.next_attribute().next_attribute().next_attribute().value();
                    length = atof(attr.next_attribute().next_attribute().value());
                    i++;
                }
                if (i > 1)
                    break;
            }
            if (i == 1)
            {
                vector<double> nums;
                stringstream ss(shape);
                double num;
                while (ss >> num)
                {
                    nums.push_back(num);
                    if (ss.peek() == ',' || ss.peek() == ' ')
                    {
                        ss.ignore();
                    }
                }

                std::vector<Point> points;

                for (int i = 0; i < nums.size() - 1; i += 2)
                {
                    points.push_back(Point({nums[i], nums[i + 1]}));
                }
                get<1>(j) = points;
                juncs.push_back(j);
            }
        }
    }

    for (Edge e : edges)
    {
        splitEdge(1.56, &e);
    }

    for (Junc j : juncs)
    {
        splitJunc(1.56, &j);
    }

    myfile.close();
    return 1;
}