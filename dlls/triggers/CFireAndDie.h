// Fires a target after level transition and then dies
class CFireAndDie : public CBaseDelay
{
public:
	void Spawn(void);
	void Precache(void);
	void Think(void);
	int ObjectCaps(void) { return CBaseDelay::ObjectCaps() | FCAP_FORCE_TRANSITION; }	// Always go across transitions
};