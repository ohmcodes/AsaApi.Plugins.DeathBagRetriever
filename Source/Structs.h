
struct PlayerDeathData
{
	FString EosID;
	unsigned long long PlayerID;
	AShooterCharacter* Corpse;
	FVector Location;

	bool operator==(const PlayerDeathData& other) const {
		return EosID == other.EosID;
	}
};