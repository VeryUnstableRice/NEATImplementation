#include "GenomeMutation.h"
#include "GenomeMutation.h"
#include "Genome.h"
#include <random>
#include <iostream>
#include <assert.h>


static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<> dis(-1, 1);
std::uniform_int_distribution<> activation_random(0, (int)eNodeActivation::Total - 1);

bool cGenomeMutation::Mutate_Link()
{
	std::vector<std::pair<std::uint32_t, std::uint32_t>> candidates;
	for (int i = 0; i < m_parent->m_nodeGenes.size(); i++)
	{
		for (int j = 0; j < m_parent->m_nodeGenes.size(); j++)
		{
			//skip if in and out are the same node
			//skip if in is an output node
			//skip if in and out are already connected
			if (i == j || m_parent->IsConnected(i, j) || m_parent->m_nodeGenes[i].Type == eNodeType::OUTPUT || m_parent->m_nodeGenes[j].Type == eNodeType::INPUT) continue;
			// Check the innovation numbers of the nodes
			int fromInnovation = m_parent->m_nodeGenes[i].Innovation;
			int toInnovation = m_parent->m_nodeGenes[j].Innovation;
			// If the from node has a smaller innovation number than the to node
			// or if the to node is an output node, add them to the candidates list
			if (fromInnovation < toInnovation || m_parent->m_nodeGenes[j].Type == eNodeType::OUTPUT)
			{
				candidates.push_back(std::make_pair(fromInnovation, toInnovation));
			}
		}
	}

	//if no candidates return
	if (candidates.empty()) return false;

	//choosing random canditate
	std::uniform_int_distribution<> distrib(0, candidates.size() - 1);
	std::pair<std::uint32_t, std::uint32_t> canditate = candidates[distrib(gen)];
	//creating the link
	m_parent->CreateLink(canditate.first, canditate.second);
	return true;
}

bool cGenomeMutation::Mutate_Node()
{

	// If there are no connection genes, do nothing
	if (m_parent->m_connectionGenes.empty()) return false;

	// Pick a random connection gene
	std::uniform_int_distribution<> distrib(0, m_parent->m_connectionGenes.size() - 1);
	int index = distrib(gen);
	sConnection_Gene& gene = m_parent->m_connectionGenes[index];

	// Disable the original connection
	gene.Disabled = true;

	std::uint32_t old_in = gene.InNode;
	std::uint32_t old_out = gene.OutNode;

	// Create a new node gene
	std::uint32_t new_innovation = m_parent->CreateNode(eNodeType::HIDDEN);


	//breaking the connection into 2
	m_parent->CreateLink(old_in, new_innovation, 1.0, 0.0);
	m_parent->CreateLink(new_innovation, old_out);
	return true;
}

bool cGenomeMutation::Mutate_Disable()
{
	if (sConnection_Gene* gene = GetRandomConnection())
	{
		gene->Disabled = !gene->Disabled;
		return true;
	}
	return false;
}

bool cGenomeMutation::Random_Shift()
{
	if (sConnection_Gene* gene = GetRandomConnection())
	{
		gene->Weight += dis(gen);
		gene->Bias += dis(gen);

		//gene->Weight = std::clamp(gene->Weight, f_type(- 1.0), f_type(1.0));
		//gene->Weight = std::clamp(gene->Bias, f_type(- 1.0), f_type(1.0));

		return true;
	}
	return false;
}

bool cGenomeMutation::Mutate_Random()
{
	if (sConnection_Gene* gene = GetRandomConnection())
	{
		gene->Weight = dis(gen);
		gene->Bias = dis(gen);
		return true;
	}
	return false;
}

bool cGenomeMutation::Mutate_RemoveLink()
{
	if (sConnection_Gene* gene = GetRandomConnection())
	{
		std::remove(m_parent->m_connectionGenes.begin(), m_parent->m_connectionGenes.end(), *gene);
	}
	return false;
}

bool cGenomeMutation::Mutate_Activation()
{
	if (sNode_Gene* gene = GetRandomNode())
	{
		gene->NodeActivation = eNodeActivation(activation_random(gen));
	}
	return false;
}

sConnection_Gene* cGenomeMutation::GetRandomConnection()
{
	if (m_parent->m_connectionGenes.empty()) return nullptr;

	std::uniform_int_distribution<> distrib(0, m_parent->m_connectionGenes.size() - 1);
	sConnection_Gene& gene = m_parent->m_connectionGenes[distrib(gen)];
	return &m_parent->m_connectionGenes[distrib(gen)];
}

sNode_Gene* cGenomeMutation::GetRandomNode()
{
	if (m_parent->m_nodeGenes.empty()) return nullptr;

	std::uniform_int_distribution<> distrib(0, m_parent->m_nodeGenes.size() - 1);
	sNode_Gene& gene = m_parent->m_nodeGenes[distrib(gen)];
	return &m_parent->m_nodeGenes[distrib(gen)];
}


void cGenomeMutation::Mutate()
{
	std::uniform_real_distribution<> chance(0, 1);

	float NodeMutationRate = std::max(0.01f, m_nodeChances / m_parent->m_nodeGenes.size());
//#define OnlyOnce
#ifdef OnlyOnce
	for (;;)
	{
		if (chance(gen) < m_removeLink)
		{
			if (Mutate_RemoveLink()) break;
		}
		if (chance(gen) < NodeMutationRate)
		{
			if (Mutate_Node()) break;
		}
		if (chance(gen) < m_nodeChances)
		{
			if (Mutate_Activation()) break;
		}
		if (chance(gen) < m_linkChances)
		{
			if (Mutate_Link()) break;
		}
		if (chance(gen) < m_disableChances)
		{
			if (Mutate_Disable()) break;
		}
		if (chance(gen) < m_randomChances)
		{
			if (Mutate_Random()) break;
		}
		if (chance(gen) < m_shiftChances)
		{
			if (Random_Shift()) break;
		}
	}
#else
	if (chance(gen) < m_removeLink)
	{
		Mutate_RemoveLink();
	}
	if (chance(gen) < NodeMutationRate)
	{
		Mutate_Node();
	}
	if (chance(gen) < m_nodeChances)
	{
		Mutate_Activation();
	}
	if (chance(gen) < m_linkChances)
	{
		Mutate_Link();
	}
	if (chance(gen) < m_disableChances)
	{
		Mutate_Disable();
	}
	if (chance(gen) < m_randomChances)
	{
		Mutate_Random();
	}
	if (chance(gen) < m_shiftChances)
	{
		Random_Shift();
	}
#endif
	m_parent->Prune();
	m_parent->SortGenes();

}
