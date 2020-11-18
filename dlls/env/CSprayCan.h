
//==============================================
// !!!UNDONE:ultra temporary SprayCan entity to apply
// decal frame at a time. For PreAlpha CD
//==============================================
class CSprayCan : public CBaseEntity
{
public:
	void	Spawn(entvars_t* pevOwner);
	void	Think(void);

	virtual int	ObjectCaps(void) { return FCAP_DONT_SAVE; }
};