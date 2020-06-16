
import subprocess

node_name = "cubesat"

def request(agent_name, req_name, *args):
	global node_name
	print("Make request '" + req_name + "' to agent '" + agent_name + "'")
	subprocess.run(["agent", node_name, agent_name, req_name] + list(args))




