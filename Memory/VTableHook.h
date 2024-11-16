#include <cstdint>  // For standard integer types
#include <cstdlib>  // For memory allocation and deallocation
#include <iostream> // for the std::memory
namespace Hook
{
    class VTableHook
    {
    public:
        VTableHook() : originalVtable(nullptr), lastHookedFunctionAddress(0), lastHookedFunctionIndex(-1) {}

        // Hook a virtual function by replacing its entry in the VTable
        template <typename FunctionType>
        bool Hook(void* instance, void* hookFunction, int vtableIndex, FunctionType* originalFunction)
        {
            // Retrieve the original VTable
            originalVtable = *reinterpret_cast<uintptr_t**>(instance);

            // Check if this function is already hooked
            if (lastHookedFunctionAddress && lastHookedFunctionIndex != -1)
            {
                if (lastHookedFunctionAddress == originalVtable[lastHookedFunctionIndex])
                {
                    return false; // Already hooked
                }
            }

            // Get the original function from the VTable
            void* originalFunctionPtr = reinterpret_cast<void*>(originalVtable[vtableIndex]);

            // If the function is not already hooked
            if (originalFunctionPtr != hookFunction)
            {
                // Calculate the number of entries in the VTable
                int tableSize = 0;
                while (originalVtable[tableSize])
                {
                    ++tableSize;
                }

                // Allocate memory for the new VTable
                uintptr_t* newVtable = reinterpret_cast<uintptr_t*>(std::malloc(tableSize * sizeof(uintptr_t)));

                // Copy the original VTable entries to the new VTable
                std::memcpy(newVtable, originalVtable, tableSize * sizeof(uintptr_t));

                // Save the original function pointer
                *originalFunction = reinterpret_cast<FunctionType>(originalFunctionPtr);

                // Hook the function by replacing the VTable entry
                newVtable[vtableIndex] = reinterpret_cast<uintptr_t>(hookFunction);

                // Set the new VTable for the instance
                *reinterpret_cast<uintptr_t**>(instance) = newVtable;

                // Update last hooked function details
                lastHookedFunctionAddress = reinterpret_cast<uintptr_t>(hookFunction);
                lastHookedFunctionIndex = vtableIndex;

                return true;
            }

            return false; // Function already hooked
        }

        // Revert a previously applied hook
        template <typename FunctionType>
        void Unhook(FunctionType originalFunction, void* instance)
        {
            // Ensure there is a valid original VTable and hook index
            if (originalVtable && lastHookedFunctionIndex != -1)
            {
                // Get the current VTable from the instance
                uintptr_t* currentVtable = *reinterpret_cast<uintptr_t**>(instance);

                // Restore the original function pointer in the VTable
                currentVtable[lastHookedFunctionIndex] = reinterpret_cast<uintptr_t>(originalFunction);

                // Restore the original VTable in the instance
                *reinterpret_cast<uintptr_t**>(instance) = originalVtable;

                // Free the memory allocated for the new VTable
                std::free(currentVtable);

                // Reset last hooked function details
                lastHookedFunctionAddress = 0;
                lastHookedFunctionIndex = -1;
            }
        }

    private:
        uintptr_t* originalVtable;
        uintptr_t lastHookedFunctionAddress;
        int lastHookedFunctionIndex;
    };
}
