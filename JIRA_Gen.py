import json
import networkx as nx
from networkx.drawing.nx_pydot import read_dot
import nltk
from nltk import word_tokenize, pos_tag
from nltk.corpus import wordnet as wn

nltk.download('punkt')
nltk.download('averaged_perceptron_tagger')
nltk.download('wordnet')

def generate_description(function_name, cfg_complexity, call_frequency):
    # Example description using NLP
    description = (f"Refactor the function '{function_name}' to improve its maintainability. "
                   f"The function has a Cyclomatic Complexity of {cfg_complexity} and has been called {call_frequency} times "
                   "in the current execution flow. Focus on reducing the complexity and ensuring that it handles edge cases effectively.")
    return description

def parse_dot_file(dot_file_path):
    graph = read_dot(dot_file_path)
    return graph

def generate_jira_story(function_name, cfg_complexity, call_frequency):
    story = {
        "summary": f"Refactor Function: {function_name}",
        "description": generate_description(function_name, cfg_complexity, call_frequency),
        "priority": "High" if cfg_complexity > 10 else "Medium",
        "labels": ["refactor", "complexity", "automation"],
        "assignee": "unassigned"
    }
    return story

def process_dot_files(cfg_dot_file, functional_flow_dot_file):
    cfg_graph = parse_dot_file(cfg_dot_file)
    flow_graph = parse_dot_file(functional_flow_dot_file)
    
    stories = []

    for node in cfg_graph.nodes:
        function_name = node
        cfg_complexity = int(cfg_graph.nodes[node].get('Complexity', 0))
        call_frequency = int(flow_graph.nodes[node].get('CallFrequency', 0))

        story = generate_jira_story(function_name, cfg_complexity, call_frequency)
        stories.append(story)

    return stories

# Sample DOT files (representing CFG and Functional Flow)
cfg_dot_file = 'cfg_graph.dot'
functional_flow_dot_file = 'functional_flow_graph.dot'

# Generate JIRA stories
jira_stories = process_dot_files(cfg_dot_file, functional_flow_dot_file)

# Output JIRA stories in JSON format
print(json.dumps(jira_stories, indent=4))

# Sample input and output
sample_dot_cfg = """
digraph CFG {
    "do_move" [Complexity="12"];
    "check_move_validity" [Complexity="7"];
    "update_position" [Complexity="4"];
}
"""

sample_dot_flow = """
digraph Flow {
    "do_move" [CallFrequency="100"];
    "check_move_validity" [CallFrequency="80"];
    "update_position" [CallFrequency="50"];
}
"""

# Simulated reading from a file for demonstration
with open('cfg_graph.dot', 'w') as f:
    f.write(sample_dot_cfg)

with open('functional_flow_graph.dot', 'w') as f:
    f.write(sample_dot_flow)

# Re-run to see the output
jira_stories = process_dot_files(cfg_dot_file, functional_flow_dot_file)
print(json.dumps(jira_stories, indent=4))
