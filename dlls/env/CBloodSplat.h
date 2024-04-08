#pragma once

class CBloodSplat : public CBaseEntity
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	void	Spawn(entvars_t* pevOwner);
	void	Spray(void);
};