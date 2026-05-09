#include "CBaseEntity.h"

#define SF_RPLR_REUSABLE 1

#define SKYBOX_MODEL_PATH "models/skybox"

// minimum distance needed to render the fake skybox (bigass model)
#define SKYBOX_MIN_ZMAX 2097152

class EXPORT CTriggerChangeSky : public CBaseEntity
{
public:
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	string_t m_skyname;
	Vector m_color;
};