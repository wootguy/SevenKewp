class CFuncPlatRot : public CFuncPlat
{
public:
	void Spawn(void);
	void SetupRotation(void);

	virtual void	GoUp(void);
	virtual void	GoDown(void);
	virtual void	HitTop(void);
	virtual void	HitBottom(void);

	void			RotMove(Vector& destAngle, float time);
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	Vector	m_end, m_start;
};