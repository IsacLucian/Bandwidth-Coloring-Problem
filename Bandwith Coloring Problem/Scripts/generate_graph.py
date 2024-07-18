import sys
import ast
import networkx as nx
import os
import matplotlib.pyplot as plt


def generate_graph(tmp_path):

    edges = []
    node_values = []
    with open(tmp_path, 'r') as f:
        line = f.readline()
        edges = ast.literal_eval(line.strip())
        line = f.readline()
        node_values = ast.literal_eval(line.strip())

    directory, filename = os.path.split(tmp_path)
    filename, _ = os.path.splitext(filename)

    G = nx.random_geometric_graph(len(node_values), 5)
    G.clear()
    G.add_nodes_from(node_values.items())
    G.add_edges_from(edges)
    
    pos = nx.spring_layout(G, scale=4)
    plt.figure(figsize=(12 + len(node_values) // 10, 10 + len(node_values) // 10))

    nx.draw(G, pos, with_labels=False)
    for node, value in node_values.items():
        x, y = pos[node]
        plt.text(x, y, s=str(value['value']), ha='center', va='center')

    edge_labels = nx.get_edge_attributes(G, 'label')
    nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, bbox={'facecolor':'white', 'edgecolor':'none', 'pad':0.5}, font_size=12)
    plt.savefig(directory + '\\' + filename + '.png', format="png")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python draw_graph.py <edges> <node_values> <name>")
        sys.exit(1)

    tmp_path = sys.argv[1]
    generate_graph(tmp_path)