#ifndef Genome_Header
#define Genome_Header
#include <vector>
#include <memory>
#include "GenomeMutation.h"

using f_type = float;

enum class eNodeType
{
	None,
	INPUT,
	HIDDEN,
	OUTPUT
};

enum class eNodeActivation
{
	Sigmoid,
	Relu,
	Tanh,
	LeakyRelu,
	Total
};

enum class eGeneType
{
	None,
	Node,
	Connection,

	Total
};

struct sGene
{
	std::uint32_t Innovation;
	eGeneType GeneType = eGeneType::None;
	virtual ~sGene() {};
};

struct sNode_Gene : public sGene
{
	eNodeType Type;
	eNodeActivation NodeActivation;

	sNode_Gene() 
	{
		Type = eNodeType::None;
		GeneType = eGeneType::Node;
	}
};

struct sConnection_Gene : public sGene
{
	std::uint32_t InNode = 0;
	std::uint32_t OutNode = 0;


	f_type Weight = 0;
	f_type Bias = 0;

	bool Disabled = false;

	bool operator==(const sConnection_Gene& gene)
	{
		return Innovation == gene.Innovation;
	}

	sConnection_Gene()
	{
		GeneType = eGeneType::Connection;
	}
};

class cGenome
{
	static std::uint32_t m_innovationNum;


	std::uint32_t m_inputNum, m_outputNum;
	std::pair<std::uint32_t, std::uint32_t> m_outputRange;
	std::vector<sNode_Gene> m_nodeGenes;
	std::vector<sConnection_Gene> m_connectionGenes;
	f_type m_fitness = 0;

	void CreateLink(std::uint32_t In, std::uint32_t Out);
	void CreateLink(std::uint32_t In, std::uint32_t Out, f_type Weight, f_type Bias);

	std::uint32_t CreateNode(eNodeType type);

	void SortGenes();
	cGenomeMutation m_genomeMutation;
	friend class cGenomeMutation;

	void Prune();

	std::uint32_t GetNewInnovation();
public:
	cGenome(int input_num, int output_num);
	cGenome(const cGenome& A);
	
	std::uint32_t GetOutputNum() const { return m_inputNum; };
	std::uint32_t GetInputNum() const { return m_outputNum; };

	cGenomeMutation& GetGenomeMutation() { return m_genomeMutation; };

	bool IsConnected(int in, int out) const;

	void SetFitness(f_type fitness);
	f_type GetFitness() const;

	const std::vector<sNode_Gene>& GetNodeGenes() const { return m_nodeGenes; }
	const std::vector<sConnection_Gene>& GetConnectionGenes() const { return m_connectionGenes; }

	static cGenome Crossover(const cGenome& firstParent, const cGenome& secondParent);
};

inline void cGenome::SetFitness(f_type fitness)
{
	m_fitness = fitness;
}

inline f_type cGenome::GetFitness() const
{
	return m_fitness;
}

#endif