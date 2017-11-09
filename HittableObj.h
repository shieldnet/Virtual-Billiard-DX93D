#ifndef _HITTABLEOBJ_
#define _HITTABLEOBJ_

#define noexcept throw ()
#define BLANK_BLOCK {}
class CSphere;

class Hittable
{
public:
	virtual bool hasIntersected(CSphere& ball) const noexcept = 0; // 겹쳤는 지(충돌했는지) 확인
	virtual void hitBy(CSphere& ball) noexcept = 0; // 충돌했는지 검사하고 공의 힘과 방향을 바꿈

	virtual ~Hittable() BLANK_BLOCK
};

#endif

