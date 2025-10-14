import math

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

    def __str__(self):
        return f'{self.v1} {self.v2} {self.v3}'

    def get_edge(self, edge_no):
        return self.e1 if edge_no == 1 else self.e2 if edge_no == 2 else self.e3

    def edge_coordinates(self, edge_no):
        edge = self.get_edge(edge_no)
        coord1 = (self.points[edge[0]].x, self.points[edge[0]].y)
        coord2 = (self.points[edge[1]].x, self.points[edge[1]].y)
        return coord1, coord2

    def vertex_coordinates(self, vertex):
        return [self.points[vertex].x, self.points[vertex].y]

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

    def set_vertex(self, vertex, value):
        self.v1 = value if vertex == 1 else self.v1
        self.v2 = value if vertex == 2 else self.v2
        self.v3 = value if vertex == 3 else  self.v3
        self.e1 = [self.v1, self.v2]
        self.e2 = [self.v2, self.v3]
        self.e3 = [self.v3, self.v1]

    def common_edge(self, other_t):
        def is_same_edge(e, t: Triangle):
            return (e[0] == t.v1 or e[0] == t.v2 or e[0] == t.v3) and (e[1] == t.v1 or e[1] == t.v2 or e[1] == t.v3)
        # this might work but makes an assumption that exists at least a common edge to two adjacent triangles in the input data
        # return self.e1 if self.e1 == other_t.e1 else self.e2 if self.e2 == other_t.e2 else self.e3
        return self.e1 if is_same_edge(self.e1, other_t) else self.e2 if is_same_edge(self.e2, other_t) else self.e3 if is_same_edge(self.e3, other_t) else None

    def opposite_vertices(self, other_t, common_edge):
        e = common_edge
        v1 = self.v1 if self.v1 not in e else self.v2 if self.v2 not in e else self.v3 if self.v3 not in e else None
        v2 = other_t.v1 if other_t.v1 not in e else other_t.v2 if other_t.v2 not in e else other_t.v3 if other_t.v3 not in e else None
        return [v1, v2]

    def __eq__(self, other):
        return self.v1 == other.v1 and self.v2 == other.v2 and self.v3 == other.v3
        pass

    def find_adjacent_triangles(self, triangles: list["Triangle"]):
        result = []
        for t in triangles:
            if self != t and self.common_edge(t):
                result.append(t)
        return result

    def flip_edges(self, other_t):
        c_e = self.common_edge(other_t)
        print(f'triangles: {self}, {other_t}')
        if self == other_t:
            print(f'triangles are equal, skip!')
            return False
        print(f'common edge: {c_e}')
        if c_e is None:
            return False
        o_vs = self.opposite_vertices(other_t, c_e)
        print(f'opposite vertices: {o_vs}')
        circumcircle = triangle_circumcircle(self.coordinates())
        def distance(a, b):
            return math.sqrt((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2)
        # skip if o_vs[1] is not in the circumcircle
        if distance(circumcircle[1], self.vertex_coordinates(o_vs[1])) >= circumcircle[0]:
            return False
        print(f'flipping...')
        self.set_vertex(1, o_vs[0])
        self.set_vertex(2, o_vs[1])
        self.set_vertex(3, c_e[1])
        other_t.set_vertex(1, o_vs[0])
        other_t.set_vertex(2, o_vs[1])
        other_t.set_vertex(3, c_e[0])
        print(f'triangles after flipping: {self}, {other_t}')
        return True


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
        padding = 10
        ax.set_xlim((0, max(ax.get_xlim()[1], ax.get_ylim()[1], coordinates[0].max() + padding)))
        ax.set_ylim((0, max(ax.get_ylim()[1], ax.get_ylim()[1], coordinates[1].max() + padding)))
        
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

points, triangles = read_data("inputTriangulation3.txt")

triangles_count = min(50, len(triangles))

flip_count = 0

def triangulation(points: list[Point], triangles: list[Triangle], circumscribed_triangle = 0, manually = False):
    Triangle.annotations = [None] * len(triangles)

    def draw_triangles(count):
        print('redraw_triangles...')
        ax.clear()
        ax.set_title(f'Triangulation({triangles_count})')
        ax.set_xlabel('x')
        ax.set_ylabel('y')
        Triangle.annotations = [None] * count
        for i, t in enumerate(triangles[:count]):
            t.draw(i, circumscribed_triangle == i)
        ax.set_aspect('equal', adjustable='box')
        fig.canvas.draw()  # refresh

    if manually and circumscribed_triangle < len(triangles):
        t = triangles[circumscribed_triangle]
        adjacent_triangles = t.find_adjacent_triangles(triangles)
        for other_t in adjacent_triangles:
            if t.flip_edges(other_t):
                global flip_count
                flip_count += 1

        draw_triangles(len(triangles))
        return

    flipped = True
    while flipped:
        flipped = False
        draw_triangles(len(triangles))
        for t in triangles[:triangles_count]:
            """
            Notice that the edges are flipped without any ordering as suggested in the lab.
            This may lead to a slightly different result, with more edges flipped than needed.
            However, it seems the optimization implementation is not that trivial and is left for latter.  
            """
            adjacent_triangles = t.find_adjacent_triangles(triangles)
            for other_t in adjacent_triangles:
                if t.flip_edges(other_t):
                    flip_count += 1
                    flipped = True
                    break


def input_changed(triangle):
    # print(triangle)
    ax.clear()
    ax.set_title(f'Triangulation({triangles_count})')
    ax.set_xlabel('x')
    ax.set_ylabel('y')

    triangulation(points, triangles, int(triangle), True)

    print(f'flip count: {flip_count}')

    ax.set_aspect('equal', adjustable='box')
    fig.canvas.draw() # refresh


# Triangulation

ax.set_title(f'Triangulation({triangles_count})')
ax.set_xlabel('x')
ax.set_ylabel("y")

triangulation(points, triangles)

print(f'flip count: {flip_count}')

# plt.axis("scaled") # !!! this discards xlim and ylim
ax.set_aspect('equal', adjustable='box') # this does the trick to have equal scale

box = fig.add_axes((0.35, 0.01, 0.03, 0.04))
text_box = TextBox(box, 'Circumcircle triangle: ', initial='0')
text_box.on_submit(input_changed)

plt.show()