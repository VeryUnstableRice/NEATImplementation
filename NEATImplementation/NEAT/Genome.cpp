#include "Genome.h"
#include <stdexcept>
#include <random>
#include <unordered_map>
#include <cassert>
#include <cmath>
#include <set>
#include <unordered_set>

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<> dis(-1, 1);
static std::uniform_int_distribution<> coinflip(0, 1);

std::uint32_t cGenome::m_innovationNum = 0;

void cGenome::SortGenes()
{
	std::sort(m_connectionGenes.begin(), m_connectionGenes.end(), [&](const sConnection_Gene& A, const sConnection_Gene& B)
		{
			if (A.OutNode >= m_outputRange.first && A.OutNode <= m_outputRange.second) // A is in the output range
			{
				if (B.OutNode >= m_outputRange.first && B.OutNode <= m_outputRange.second) // B is also in the output range
					return A.Innovation < B.Innovation; // sort by innovation
				else
					return false; // A should be sorted last
			}
			else // A is not in the output range
			{
				if (B.OutNode >= m_outputRange.first && B.OutNode <= m_outputRange.second) // B is in the output range
					return true; // A should be sorted first
				else // B is also not in the output range
				{
					if (A.OutNode == B.OutNode) // same OutNode
						return A.Innovation < B.Innovation; // sort by innovation
					else
						return A.OutNode < B.OutNode; // sort by OutNode
				}
			}
		});
	std::sort(m_nodeGenes.begin(), m_nodeGenes.end(), [](const sNode_Gene& A, const sNode_Gene& B) { return A.Innovation < B.Innovation; });
}

cGenome::cGenome(int input_num, int output_num) : m_inputNum(input_num), m_outputNum(output_num), m_genomeMutation(this)
{
	std::uint32_t total_num = input_num + output_num;
	m_nodeGenes.reserve(total_num);

	for (int i = 0; i < input_num; ++i)
	{
		CreateNode(eNodeType::INPUT);
	}

	for (int i = 0; i < output_num; ++i)
	{
		int innovation = CreateNode(eNodeType::OUTPUT);
		if (i == 0)
		{
			m_outputRange.first = innovation;
		}
		if (i == output_num - 1)
		{
			m_outputRange.second = innovation;
		}
	}
}

cGenome::cGenome(const cGenome& A) : m_genomeMutation(this)
{
	m_outputRange		= A.m_outputRange;
	m_connectionGenes	= A.m_connectionGenes;
	m_nodeGenes			= A.m_nodeGenes;
	m_inputNum			= A.m_inputNum;
	m_outputNum			= A.m_outputNum;
	m_fitness			= A.m_fitness;
}

void cGenome::Prune()
{
	std::unordered_set<int> connectedNodes;
	// Loop through all the connection genes and add the input and output nodes to the set
	for (const auto& connectionGene : m_connectionGenes) {
		connectedNodes.insert(connectionGene.InNode);
		connectedNodes.insert(connectionGene.OutNode);
	}

	// Loop through all the node genes and remove any nodes that are not in the set
	auto it = std::remove_if(m_nodeGenes.begin(), m_nodeGenes.end(),
		[&](const sNode_Gene& nodeGene) {
			return nodeGene.Type == eNodeType::HIDDEN &&
				connectedNodes.find(nodeGene.Innovation) == connectedNodes.end();
		});
	m_nodeGenes.erase(it, m_nodeGenes.end());

	// Remove all extra connections
	m_connectionGenes.erase(std::remove_if(m_connectionGenes.begin(), m_connectionGenes.end(),
		[&](const sConnection_Gene& connection) {
			bool inNodeFound = false;
			bool outNodeFound = false;
			for (const auto& node : m_nodeGenes) {
				if (node.Innovation == connection.InNode) {
					inNodeFound = true;
				}
				if (node.Innovation == connection.OutNode) {
					outNodeFound = true;
				}
				if (inNodeFound && outNodeFound) {
					break;
				}
			}
			return !inNodeFound || !outNodeFound;
		}), m_connectionGenes.end());
}

void cGenome::CreateLink(std::uint32_t In, std::uint32_t Out)
{
	CreateLink(In, Out, dis(gen), dis(gen));
}

void cGenome::CreateLink(std::uint32_t In, std::uint32_t Out, f_type Weight, f_type Bias)
{
	sConnection_Gene connection;
	connection.InNode = In;
	connection.OutNode = Out;
	connection.Weight = Weight;
	connection.Bias = Bias;
	connection.Disabled = false;
	connection.Innovation = GetNewInnovation();
	m_connectionGenes.push_back(std::move(connection));
}

std::uint32_t cGenome::CreateNode(eNodeType type)
{
	std::uint32_t innovation = GetNewInnovation();

	sNode_Gene node;
	node.Type = type;
	node.Innovation = innovation;

	std::uniform_int_distribution<> activation_random(0, (int)eNodeActivation::Total-1);
	node.NodeActivation = eNodeActivation(activation_random(gen));
	m_nodeGenes.push_back(std::move(node));
	return innovation;
}

std::uint32_t cGenome::GetNewInnovation()
{
	return m_innovationNum++;
}

cGenome cGenome::Crossover(const cGenome& firstParent, const cGenome& secondParent)
{
	const std::vector<sNode_Gene>& nodeGenes1st = firstParent.GetNodeGenes();
	const std::vector<sNode_Gene>& nodeGenes2nd = secondParent.GetNodeGenes();
	const std::vector<sConnection_Gene>& connectionGenes1st = firstParent.GetConnectionGenes();
	const std::vector<sConnection_Gene>& connectionGenes2nd = secondParent.GetConnectionGenes();
	f_type firstParentFitness = firstParent.GetFitness();
	f_type secondParentFitness = secondParent.GetFitness();

	// Crossover node genes
	std::vector<sNode_Gene> childNodeGenes;
	childNodeGenes.reserve(nodeGenes1st.size() + nodeGenes2nd.size());
	std::set<std::uint32_t> nodeInnovations;
	// Create child node genes
	std::unordered_map<int, int> innovationToIndexMap;

	// Iterate over the nodes in the first parent's node genes
	for (const auto& node : nodeGenes1st) {
		// Use a coinflip to decide whether to add the node from the first or second parent
		const bool useFirstParentNode = coinflip(gen) == 0;
		if (useFirstParentNode) {
			// Add the node from the first parent
			childNodeGenes.push_back(node);
		}
		else {
			// Look up the node from the second parent using the innovation-to-index map
			const auto it = innovationToIndexMap.find(node.Innovation);
			if (it != innovationToIndexMap.end()) {
				childNodeGenes.push_back(nodeGenes2nd[it->second]);
			}
			else {
				// If the innovation was not found in the map, the node is a new one, so add it to the child
				childNodeGenes.push_back(node);
			}
		}
		// Add the node's innovation to the set of innovations in the child
		nodeInnovations.insert(node.Innovation);
		// Add the node's index to the innovation-to-index map
		innovationToIndexMap[node.Innovation] = childNodeGenes.size() - 1;
	}

	for (const auto& node : nodeGenes2nd) {
		if (nodeInnovations.find(node.Innovation) == nodeInnovations.end()) {
			childNodeGenes.push_back(node);
		}
	}

	// Crossover connection genes
	std::vector<sConnection_Gene> childConnectionGenes;
	childConnectionGenes.reserve(connectionGenes1st.size() + connectionGenes2nd.size());
	std::set<std::pair<std::uint32_t, std::uint32_t>> connectionInnovations;
	for (const auto& connection : connectionGenes1st) {
		if (connectionGenes2nd.size() == 0 || connectionInnovations.find({ connection.InNode, connection.OutNode }) != connectionInnovations.end() ||
			(connectionGenes2nd.size() > 0 && connection.Innovation > connectionGenes2nd.back().Innovation)) {
			childConnectionGenes.push_back(connection);
			connectionInnovations.insert({ connection.InNode, connection.OutNode });
		}
		else {
			for (const auto& otherConnection : connectionGenes2nd) {
				if (otherConnection.InNode == connection.InNode && otherConnection.OutNode == connection.OutNode) {
					childConnectionGenes.push_back(coinflip(gen) == 0 ? connection : otherConnection);
					connectionInnovations.insert({ connection.InNode, connection.OutNode });
					break;
				}
				else if (connection.Innovation < otherConnection.Innovation) {
					childConnectionGenes.push_back(connection);
					connectionInnovations.insert({ connection.InNode, connection.OutNode });
					break;
				}
			}
		}
	}
	if (connectionGenes2nd.size() > 0) {
		for (const auto& connection : connectionGenes2nd) {
			if (connectionInnovations.find({ connection.InNode, connection.OutNode }) == connectionInnovations.end()) {
				childConnectionGenes.push_back(connection);
				connectionInnovations.insert({ connection.InNode, connection.OutNode });
			}
		}
	}

	// Construct child genome
	cGenome child(firstParent);
	child.m_nodeGenes = std::move(childNodeGenes);
	child.m_connectionGenes = std::move(childConnectionGenes);

	return child;
}

bool cGenome::IsConnected(int in, int out) const
{
	for (const sConnection_Gene& gene : m_connectionGenes)
	{
		if (!gene.Disabled && gene.InNode == in && gene.OutNode == out)
			return true;
	}
	return false;
}