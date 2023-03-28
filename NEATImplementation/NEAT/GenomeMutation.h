#ifndef GenomeMutation_Header
#define GenomeMutation_Header


class cGenomeMutation
{
	class cGenome* m_parent;

	bool Mutate_Link();
	bool Mutate_Node();
	bool Mutate_Disable();
	bool Random_Shift();
	bool Mutate_Random();
	bool Mutate_RemoveLink();
	bool Mutate_Activation();

	float m_linkChances = 0.03f;
	float m_nodeChances = 0.3f;
	float m_disableChances = 0.3f;
	float m_shiftChances = 0.3f;
	float m_randomChances = 0.3f;
	float m_removeLink = 0.3f;
	float m_changeActivation = 0.3;


	struct sConnection_Gene* GetRandomConnection();
	struct sNode_Gene* GetRandomNode();
public:
	void Mutate();

	cGenomeMutation(cGenome* genome) : m_parent(genome) {};
};

#endif