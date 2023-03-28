#include "NeuralNetwork.h"
#include "Genome.h"
#include <unordered_map>
#include <iostream>

cNeuralNetwork::cNeuralNetwork(cGenome* genome)
{
	m_parent = genome;
	Initialize();
}

std::vector<f_type> cNeuralNetwork::Evaluate(const std::vector<f_type>& inputs)
{
	//reseting the neural network
	ResetNeuralNetwork();

	//preparing inputs
	PrepareInputs(inputs);

	//calculate
	Calculate();

	//output
	return PrepareOutput();
}

void cNeuralNetwork::PrepareInputs(const std::vector<f_type>& inputs)
{
	for (int i = 0; i < inputs.size(); ++i)
	{
		m_nodes[i].Value = inputs[i];
	}
}

std::vector<f_type> cNeuralNetwork::PrepareOutput()
{
	std::vector<f_type> output;
	output.reserve(m_parent->GetOutputNum());
	for (int i = 0; i < m_nodes.size(); ++i)
	{
		if (m_nodes[i].Gene->Type == eNodeType::OUTPUT)
			output.push_back(m_nodes[i].Value);
		if (output.size() == m_parent->GetOutputNum())
			break;
	}
	return output;
}

void cNeuralNetwork::Calculate()
{
	if (m_connections.empty())
		return;
	sNode* LastNode = m_connections[0].Out;
	LastNode->Value = 0; // reset node value
	for (sConnection& connection : m_connections)
	{
		sConnection_Gene* Gene = connection.Gene;
		if (LastNode != connection.Out)
		{
			ActivateNode(LastNode);
			LastNode = connection.Out;
			LastNode->Value = 0; // reset node value
		}
		connection.Out->Value += connection.In->Value * Gene->Weight + Gene->Bias;
	}

	ActivateNode(LastNode);
}
void cNeuralNetwork::ActivateNode(sNode* LastNode)
{
	switch (LastNode->Gene->NodeActivation)
	{
	case eNodeActivation::Sigmoid:
		LastNode->Value = Sigmoid(LastNode->Value);
		break;

	case eNodeActivation::Relu:
		LastNode->Value = Relu(LastNode->Value);
		break;

	case eNodeActivation::Tanh:
		LastNode->Value = tanh(LastNode->Value);
		break;

	case eNodeActivation::LeakyRelu:
		LastNode->Value = LeakyRelu(LastNode->Value);
		break;
	}
}

void cNeuralNetwork::Initialize()
{
	const std::vector<sNode_Gene>& nodeGenes = m_parent->GetNodeGenes();
	const std::vector<sConnection_Gene>& connectionGenes = m_parent->GetConnectionGenes();

	std::unordered_map<std::uint32_t, std::uint32_t> Nodes;
	for (int i = 0; i < nodeGenes.size(); ++i)
	{
		sNode node;
		node.Gene = (sNode_Gene*)&nodeGenes[i];
		m_nodes.push_back(std::move(node));
		Nodes[node.Gene->Innovation] = (std::uint32_t)m_nodes.size() - 1;
	}

	for (int i = 0; i < connectionGenes.size(); ++i)
	{
		if (connectionGenes[i].Disabled) continue;

		sConnection connection;
		connection.Gene = (sConnection_Gene*)&connectionGenes[i];

		std::uint32_t inNode = connection.Gene->InNode;
		std::uint32_t outNode = connection.Gene->OutNode;

		connection.In = &m_nodes[Nodes[inNode]];
		connection.Out = &m_nodes[Nodes[outNode]];

		m_connections.push_back(std::move(connection));
	}
}

void cNeuralNetwork::ResetNeuralNetwork()
{
	for (sNode& node : m_nodes)
		node.Value = 0;
}
