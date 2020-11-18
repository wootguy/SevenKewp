
class CRuleEntity : public CBaseEntity
{
public:
	void	Spawn(void);
	void	KeyValue(KeyValueData* pkvd);
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	void	SetMaster(int iszMaster) { m_iszMaster = iszMaster; }

protected:
	BOOL	CanFireForActivator(CBaseEntity* pActivator);

private:
	string_t	m_iszMaster;
};

// 
// CRulePointEntity -- base class for all rule "point" entities (not brushes)
//
class CRulePointEntity : public CRuleEntity
{
public:
	void		Spawn(void);
};

// 
// CRuleBrushEntity -- base class for all rule "brush" entities (not brushes)
// Default behavior is to set up like a trigger, invisible, but keep the model for volume testing
//
class CRuleBrushEntity : public CRuleEntity
{
public:
	void		Spawn(void);

private:
};
