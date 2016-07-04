/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron <http://www.natron.fr/>,
 * Copyright (C) 2016 INRIA and Alexandre Gauthier-Foichat
 *
 * Natron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Natron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
 * ***** END LICENSE BLOCK ***** */

#ifndef PRECOMPNODE_H
#define PRECOMPNODE_H

// ***** BEGIN PYTHON BLOCK *****
// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>
// ***** END PYTHON BLOCK *****

#include "Global/Macros.h"

#if !defined(Q_MOC_RUN) && !defined(SBK_RUN)
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#endif

#include "Engine/OutputEffectInstance.h"
#include "Engine/EngineFwd.h"

NATRON_NAMESPACE_ENTER;

struct PrecompNodePrivate;
class PrecompNode
    : public EffectInstance
{
GCC_DIAG_SUGGEST_OVERRIDE_OFF
    Q_OBJECT
GCC_DIAG_SUGGEST_OVERRIDE_ON

private: // derives from EffectInstance
    // constructors should be privatized in any class that derives from boost::enable_shared_from_this<>
    PrecompNode(const NodePtr& n);

public:
    static EffectInstancePtr create(const NodePtr& node) WARN_UNUSED_RETURN
    {
        return EffectInstancePtr( new PrecompNode(node) );
    }

    virtual ~PrecompNode();

    virtual int getMajorVersion() const OVERRIDE FINAL WARN_UNUSED_RETURN
    {
        return 1;
    }

    virtual int getMinorVersion() const OVERRIDE FINAL WARN_UNUSED_RETURN
    {
        return 0;
    }

    virtual int getMaxInputCount() const OVERRIDE WARN_UNUSED_RETURN
    {
        return 0;
    }

    virtual std::string getPluginID() const OVERRIDE WARN_UNUSED_RETURN;
    virtual std::string getPluginLabel() const OVERRIDE WARN_UNUSED_RETURN;
    virtual std::string getPluginDescription() const OVERRIDE WARN_UNUSED_RETURN;
    virtual void getPluginGrouping(std::list<std::string>* grouping) const OVERRIDE FINAL;
    virtual bool isInputOptional(int /*inputNb*/) const OVERRIDE
    {
        return false;
    }

    virtual void addAcceptedComponents(int inputNb, std::list<ImageComponents>* comps) OVERRIDE FINAL;
    virtual void addSupportedBitDepth(std::list<ImageBitDepthEnum>* depths) const OVERRIDE FINAL;

    ///Doesn't really matter here since it won't be used (this effect is always an identity)
    virtual RenderSafetyEnum renderThreadSafety() const OVERRIDE FINAL WARN_UNUSED_RETURN
    {
        return eRenderSafetyFullySafeFrame;
    }

    virtual bool isOutput() const OVERRIDE WARN_UNUSED_RETURN
    {
        return false;
    }

    virtual bool isHostChannelSelectorSupported(bool* /*defaultR*/,
                                                bool* /*defaultG*/,
                                                bool* /*defaultB*/,
                                                bool* /*defaultA*/) const OVERRIDE FINAL
    {
        return false;
    }

    NodePtr getOutputNode() const;

    void getPrecompInputs(NodesList* nodes) const;

    AppInstancePtr getPrecompApp() const;
    virtual bool getCreateChannelSelectorKnob() const OVERRIDE FINAL WARN_UNUSED_RETURN { return false; }

public Q_SLOTS:

    void onPreRenderFinished();

    void onReadNodePersistentMessageChanged();

private:

    virtual void initializeKnobs() OVERRIDE FINAL;
    virtual void onKnobsLoaded() OVERRIDE FINAL;
    virtual bool knobChanged(const KnobIPtr& k,
                             ValueChangedReasonEnum reason,
                             ViewSpec view,
                             double time,
                             bool originatedFromMainThread) OVERRIDE FINAL;
    boost::scoped_ptr<PrecompNodePrivate> _imp;
};

inline PrecompNodePtr
toPrecompNode(const EffectInstancePtr& effect)
{
    return boost::dynamic_pointer_cast<PrecompNode>(effect);
}

NATRON_NAMESPACE_EXIT;

#endif // PRECOMPNODE_H
