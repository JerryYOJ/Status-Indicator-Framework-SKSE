#pragma once

namespace RE {
	class FloatingQuestMarker : public HUDObject
	{
	public:
		NiPoint2 TopLeftCornerPos;       
		NiPoint2 TopRightCornerPos;
		NiPoint2 BottemLeftCornerPos;
		NiPoint2 BottemRightCornerPos;
		NiPoint2 TopLeftCornerPos_;
		NiPoint2 TopRightCornerPos_;
		NiPoint2 BottemLeftCornerPos_;
		NiPoint2 BottemRightCornerPos_;
		float centerX;
		float centerY;
		float pixelXMin;
		float pixelYMin;
		float pixelXMax;
		float pixelYMax;
		float baseClipOffsetX;
		float baseClipOffsetY;
		GFxValue hudElementsArray;
		BSTArray<GFxValue> markerClips;
	};
	static_assert(sizeof(FloatingQuestMarker) == 0xB8);

}
