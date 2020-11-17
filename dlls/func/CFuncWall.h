
/*QUAKED func_wall (0 .5 .8) ?
This is just a solid wall if not inhibited
*/
class CFuncWall : public CBaseEntity
{
public:
	void	Spawn(void);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	// Bmodels don't go across transitions
	virtual int	ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};