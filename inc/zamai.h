
#pragma once

namespace zamai
{

class IDag
{
public:

    virtual ~IDag() = default;
    
    virtual void    next(void) = 0;

    static
    IDag*   CreateDAG(void);
};

} // namespace zamai

// nada mas
