#include "Clipboard.h"

void Clipboard::RemovePayload(const std::string& data)
{
    for (auto it = mSelected.begin(); it != mSelected.end(); ++it)
    {
        if (data.compare(*it) == 0)
        {
            mSelected.erase(it);
            break;
        }
    }
}

void Clipboard::Clear(void)
{
    mSelectionType = PayloadType::Nothing;
    mSelected.clear();
}

bool Clipboard::IsStored(const std::string& data)
{
    for (let stored : mStorage)
    {
        if (stored.compare(data) == 0)
            return true;
    }
    return false;
}

void Clipboard::StoreSelection(Operation operation)
{
    mOperation = operation;
    mStoredType = mSelectionType;
    mStorage.clear();
    for (let selection : mSelected)
        mStorage.push_back(selection);
}

bool Clipboard::IsSelected(const std::string& data) const
{
    for (let selection : mSelected)
    {
        if (selection.compare(data) == 0)
            return true;
    }
    return false;
}

void Clipboard::ResetPayload(const std::string& data)
{
    mSelected.clear();
    mSelected.push_back(data);
}

Clipboard& Clipboard::Get(void)
{
    static Clipboard sClipboard;
    return sClipboard;
}
