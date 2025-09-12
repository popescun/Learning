import numpy as np
from matplotlib import pyplot as plt
from matplotlib.widgets import TextBox
from Voronoi_Diagrams_And_Delaunay_Triangulations.triangle_circumcircle import triangle_circumcircle

fig, ax = plt.subplots()

class Point:
    def __init__(self, data: str):
        numbers = data.split()
        self.id = int(numbers[0])
        self.x = int(numbers[1])
        self.y = int(numbers[2])
        

class Triangle:
    # helpful for drawing
    v1_va = 0  # initial annotations alignment
    v2_va = 0  # initial annotations alignment
    v3_va = 0  # initial annotations alignment
    annotations = []

    def __init__(self, data: str, points: list[Point]):
        numbers = data.split()
        self.v1 = int(numbers[0])
        self.v2 = int(numbers[1])
        self.v3 = int(numbers[2])
        self.e1 = [self.v1, self.v2]
        self.e2 = [self.v2, self.v3]
        self.e3 = [self.v3, self.v1]
        self.points = points

    def get_edge(self, edge_no):
        return self.e1 if edge_no == 1 else self.e2 if edge_no == 2 else self.e3

    def edge_coordinates(self, edge_no):
        edge = self.get_edge(edge_no)
        coord1 = (self.points[edge[0]].x, self.points[edge[0]].y)
        coord2 = (self.points[edge[1]].x, self.points[edge[1]].y)
        return coord1, coord2

    def coordinates(self):
        """
        :return: (2, 3) numpy matrix 
        """
        e1_coordinates = self.edge_coordinates(1)
        e2_coordinates = self.edge_coordinates(2)
        return np.matrix([e1_coordinates[0], e1_coordinates[1], e2_coordinates[1]]).transpose()

    def edge_vertices(self, edge_no):
        edge = self.get_edge(edge_no)
        return edge[0], edge[1]

    def annotate_coordinates(self, vertex):
        """ Process coordinates for multiple vertical annotations.
        
        :param vertex: vertex id
        :return: annotation coordinate
        """
        Triangle.annotations[vertex] = [self.points[vertex].x, self.points[vertex].y]  if Triangle.annotations[vertex] is None else Triangle.annotations[vertex]
        Triangle.annotations[vertex][1] -= 15
        return Triangle.annotations[vertex]

    def edge_data(self, edge):
        x_data = (self.points[edge[0]].x, self.points[edge[1]].x)
        y_data = (self.points[edge[0]].y, self.points[edge[1]].y)
        return x_data, y_data

    def draw(self, triangle_index, circumscribed: bool):
        coord = (self.points[self.v1].x + self.points[self.v2].x) / 2 + 2, (self.points[self.v1].y + self.points[self.v2].y) / 2
        ax.annotate(f't{triangle_index}', coord)

        ax.plot(*self.edge_data(self.e1), linewidth=2 if circumscribed else 0.5)
        ax.annotate(f'v{self.v1}', self.edge_coordinates(1)[0])

        ax.plot(*self.edge_data(self.e2), linewidth=2 if circumscribed else 0.5)
        ax.annotate(f'v{self.v2}',  self.edge_coordinates(1)[1])

        ax.plot(*self.edge_data(self.e3), linewidth=2 if circumscribed else 0.5)
        ax.annotate(f'v{self.v3}', self.edge_coordinates(2)[1])

        coordinates = self.coordinates()

        # set the axes limits before drawing the circumcircle
        ax.set_xlim((0, max(ax.get_xlim()[1], ax.get_ylim()[1], coordinates[0].max())))
        ax.set_ylim((0, max(ax.get_ylim()[1], ax.get_ylim()[1], coordinates[1].max())))
        
        # from https://people.math.sc.edu/Burkardt/py_src/geometry/triangle_circumcircle.py
        if circumscribed:
            circumcircle = triangle_circumcircle(coordinates)
            ax.add_patch(
                plt.Circle((circumcircle[1][0], circumcircle[1][1]), circumcircle[0], fill=False, color='r', linewidth=2))


def read_data(filename):
    with open(filename, 'r') as f:
        lines = f.read().splitlines()
        n, m = lines.pop(0).split()
        print(n, m)
        data = np.array(lines)
        n = int(n)
        m = int(m)
        points_data = data[:n]
        triangles_data = data[n:]
        points: list[Point] = []
        triangles: list[Triangle] = []
        for p in points_data:
            points.append(Point(p))
        for t in triangles_data:
            triangles.append(Triangle(t, points))
    return points, triangles


def triangulation(points: list[Point], triangles: list[Triangle], circumscribed_triangle):
    Triangle.annotations = [None] * len(triangles)
    for i, t in enumerate(triangles[:]):
        t.draw(i, circumscribed_triangle == i)
    pass

points, triangles = read_data("inputTriangulation1.txt")


def input_changed(triangle):
    # print(triangle)
    ax.clear()
    ax.set_title("Triangulation")
    ax.set_xlabel("x")
    ax.set_ylabel("y")

    triangulation(points, triangles, int(triangle))

    ax.set_aspect('equal', adjustable='box')
    fig.canvas.draw()


# Triangulation

ax.set_title("Triangulation")
ax.set_xlabel("x")
ax.set_ylabel("y")

triangulation(points, triangles, 0)

# plt.axis("scaled") # !!! this discards xlim and ylim
ax.set_aspect('equal', adjustable='box') # this does the trick to have equal scale

box = fig.add_axes((0.3, 0.01, 0.03, 0.04))
text_box = TextBox(box, "Triangle ", initial="0")
text_box.on_submit(input_changed)

plt.show()