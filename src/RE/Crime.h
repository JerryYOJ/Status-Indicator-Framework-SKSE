namespace RE {
	struct Crime
	{
	public:
		// members
		std::uint32_t           refCount;           // 00
		PackageNS::CRIME_TYPE   crimeType;          // 04
		ObjectRefHandle         sceneHandle;        // 08 - scene/victim ref
		ActorHandle             criminalHandle;     // 0C
		std::uint8_t            unk10;              // 10
		std::uint8_t            pad11[7];           // 11
		TESForm* stolenItem;                        // 18 - theft/pickpocket only, null otherwise
		std::uint32_t           itemCount;          // 20 - stolen item count or total gold value
		std::uint32_t           unk24;              // 24
		BSTArray<ActorHandle>   actorsKnowOfCrime;  // 28
		TESForm* owner;                             // 40 - owner of stolen item (theft only)
		std::uint32_t           crimeID;            // 48 - unique serial ID from ProcessLists
		std::uint8_t            unk4C;              // 4C - status flag
		std::uint8_t            pad4D[3];           // 4D
		std::uint32_t           packedDate;         // 50 - packed game date (day | month<<9 | year<<13)
		float                   gameTimeElapsed;    // 54 - g_fGameTimeElapsed at time of crime
		std::uint32_t           fineAmount;         // 58 - gold bounty for this crime
		std::uint32_t           unk5C;              // 5C
		TESFaction* crimeFaction;                   // 60
		std::uint8_t            unk68;              // 68
		std::uint8_t            pad69;              // 69
		std::uint16_t           unk6A;              // 6A
		mutable BSReadWriteLock lock;               // 6C
		std::uint32_t           unk74;              // 74
	};
	static_assert(sizeof(Crime) == 0x78);
}