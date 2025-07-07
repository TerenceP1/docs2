import osmnx as ox

G = ox.graph_from_place("Manhattan, New York, USA", network_type="walk")

# Add travel times
G = ox.add_edge_speeds(G)
G = ox.add_edge_travel_times(G)

for u, v, data in G.edges(data=True):
    print(u, v, data['length'], data['travel_time'], data.get('name'))