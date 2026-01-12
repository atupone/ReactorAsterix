#pragma once

// System headers
#include <exception>

/**
 * @class notEnoughData
 * @brief Exception thrown when a data block is incomplete.
 *
 * This is used to signal that the packet or data buffer
 * ended prematurely, making it impossible to read all expected data.
 */
class notEnoughData : public std::exception {
};

/**
 * @class uninterpretedItem
 * @brief Exception thrown for an unhandled data item.
 *
 * This exception indicates that a particular data item in an ASTERIX
 * packet was not recognized or implemented by the handler.
 */
class uninterpretedItem : public std::exception {
};

/**
 * @class mandatoryItemMissing
 * @brief Exception for when a mandatory data item is not found.
 *
 * This exception is thrown when a required data item, as defined by the
 * ASTERIX category specification, is missing from the packet.
 */
class mandatoryItemMissing : public std::exception {
    private:
        mandatoryItemMissing();
        const char *missingItem;
    public:
        mandatoryItemMissing(const char *_missingItem)
            : missingItem(_missingItem) {};
        const char *what() const throw() {
            return missingItem;
        }
};


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
