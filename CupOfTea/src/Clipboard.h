#pragma once

#include <GreenTea.h>

enum class PayloadType : byte {
	Nothing = 0,
	Entities,
	BrowserItems,
	Components
};

class Clipboard {
public:

	enum class Operation { None = 0, Cut, Copy };

public:

	void SetStorageOperation(Operation operation) { mOperation = operation; }
	void StoreSelection(Operation operation);
	bool IsStored(const std::string& data);

	bool IsSelected(const std::string& data) const;
	void RemovePayload(const std::string& data);
	void ResetPayload(const std::string& data);
	void AddPayload(const std::string& data) { mSelected.push_back(data); }
	void Clear(void);

	void SetPayloadType(PayloadType type) { mSelectionType = type; }
	inline PayloadType GetPayloadType(void) const { return mSelectionType; }

	inline size_t GetSelectedNumber(void) const { return mSelected.size(); }
	inline std::vector<std::string>& GetSelection(void) { return mSelected; }
	inline const std::vector<std::string>& GetSelection(void) const { return mSelected; }

	inline PayloadType GetStorageType(void) const { return mStoredType; }
	inline std::vector<std::string>& GetStorage(void) { return mStorage; }
	inline const std::vector<std::string>& GetStorage(void) const { return mStorage; }

	inline Operation GetOperation(void) { return mOperation; }

	static Clipboard& Get(void);

private:

	Clipboard(void) = default;

private:

	Operation mOperation = Operation::None;

	PayloadType mSelectionType = PayloadType::Nothing;
	std::vector<std::string> mSelected;

	PayloadType mStoredType = PayloadType::Nothing;
	std::vector<std::string> mStorage;

};
