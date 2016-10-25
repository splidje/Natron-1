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

#ifndef NATRON_GUI_ANIMATION_MODULE_UNDO_REDO_H
#define NATRON_GUI_ANIMATION_MODULE_UNDO_REDO_H

// ***** BEGIN PYTHON BLOCK *****
// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>
// ***** END PYTHON BLOCK *****

#include "Global/Macros.h"

#include <list>
#include <vector>
#include <map>

#if !defined(Q_MOC_RUN) && !defined(SBK_RUN)
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#endif

CLANG_DIAG_OFF(deprecated)
CLANG_DIAG_OFF(uninitialized)
#include <QtCore/QCoreApplication>
#include <QUndoCommand> // in QtGui on Qt4, in QtWidgets on Qt5
CLANG_DIAG_ON(deprecated)
CLANG_DIAG_ON(uninitialized)

#include "Engine/AnimatingObjectI.h"
#include "Engine/Curve.h"
#include "Engine/Transform.h"
#include "Engine/Variant.h"

#include "Gui/AnimItemBase.h"

NATRON_NAMESPACE_ENTER;

struct AnimItemDimViewIndexIDWithCurve
{
    AnimItemDimViewIndexID key;
    CurvePtr oldCurveState;

    AnimItemDimViewIndexIDWithCurve()
    : key()
    , oldCurveState()
    {

    }
};

struct AnimItemDimViewIndexIDWithCurve_CompareLess
{
    bool operator()(const AnimItemDimViewIndexIDWithCurve& lhs, const AnimItemDimViewIndexIDWithCurve& rhs) const
    {

        if (lhs.key.view < rhs.key.view) {
            return true;
        } else if (lhs.key.view > rhs.key.view) {
            return false;
        } else {
            if (lhs.key.dim < rhs.key.dim) {
                return true;
            } else if (lhs.key.dim > rhs.key.dim) {
                return false;
            } else {
                return lhs.key.item < rhs.key.item;
            }
        }

    }
};
typedef std::map<AnimItemDimViewIndexIDWithCurve, KeyFrameWithStringSet, AnimItemDimViewIndexIDWithCurve_CompareLess> KeysWithOldCurveMap;


/**
 * @class An undo command that can add or remove keyframes on multiple curves at once
 **/
class AddOrRemoveKeysCommand
    : public QUndoCommand
{



class AddKeysCommand : public QUndoCommand
{
    
    Q_DECLARE_TR_FUNCTIONS(AddKeysCommand)

public:
    
    /**
     * @brief Add keyframes to curves in keys
     * @param replaceExistingAnimation If true, any animation will be wiped
     **/
    AddKeysCommand(const AnimItemDimViewKeyFramesMap & keys,
                   QUndoCommand *parent = 0);
    
    virtual void undo() OVERRIDE FINAL;
    virtual void redo() OVERRIDE FINAL;
    
private:
    
    AnimItemDimViewKeyFramesMap _keys;
};

class PasteKeysCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(PasteKeysCommand)

public:
    /**
     * @brief Add keyframes in the curves in keys to the dst anim items
     **/
    PasteKeysCommand(const AnimItemDimViewKeyFramesMap & keys,
                     const std::list<AnimItemBasePtr>& dstAnim,
                     bool pasteRelativeToCurrentTime,
                     double currentTime,
                     QUndoCommand *parent = 0);
    
    virtual void undo() OVERRIDE FINAL;
    virtual void redo() OVERRIDE FINAL;
private:
    
    double _offset;
    std::list<AnimItemBasePtr> _targets;
    AnimItemDimViewKeyFramesMap _keys;
};

class RemoveKeysCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(RemoveKeysCommand)

public:
    /**
     * @brief Remove keyframes to curves in keys
     * @param replaceExistingAnimation If true, any animation will be wiped
     **/
    RemoveKeysCommand(const AnimItemDimViewKeyFramesMap & keys,
                     QUndoCommand *parent = 0);
    
    virtual void undo() OVERRIDE FINAL;
    virtual void redo() OVERRIDE FINAL;
    
private:
    
    AnimItemDimViewKeyFramesMap _keys;

    
};

/**
 * @class An undo command that can set all keyframes on multiple curves at once. Existing keyframes are deleted (and restored in undo)
 **/
class SetKeysCommand : public QUndoCommand
{

    Q_DECLARE_TR_FUNCTIONS(SetKeysCommand)

public:




    SetKeysCommand(const AnimItemDimViewKeyFramesMap & keys,
                   QUndoCommand *parent = 0);


    virtual ~SetKeysCommand() OVERRIDE
    {
    }

private:


    virtual void undo() OVERRIDE FINAL;
    virtual void redo() OVERRIDE FINAL;

private:

    KeysWithOldCurveMap _keys;
};


/**
 * @class An undo command that can warp multiple keyframes of multiple curves
 **/
class WarpKeysCommand
    : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(WarpKeysCommand)

public:

    /**
     * @brief Moves given keyframes, nodes and items by dt and dv.
     **/
    WarpKeysCommand(const AnimItemDimViewKeyFramesMap& keys,
                    const std::vector<NodeAnimPtr >& nodes,
                    const std::vector<TableItemAnimPtr>& tableItems,
                    double dt,
                    double dv,
                    QUndoCommand *parent = 0);

    /**
     * @brief Ctor that warps using an affine transformation. Cannot transform nodes and table items
     **/
    WarpKeysCommand(const AnimItemDimViewKeyFramesMap& keys,
                    const Transform::Matrix3x3& matrix,
                    QUndoCommand *parent = 0);
    
    virtual ~WarpKeysCommand() OVERRIDE
    {
    }

    /**
     * @brief Attempts to warp the given keys and return true on success
     **/
    static bool testWarpOnKeys(const AnimItemDimViewKeyFramesMap& inKeys, const Curve::KeyFrameWarp& warp);
    
private:

    virtual void undo() OVERRIDE FINAL;
    virtual void redo() OVERRIDE FINAL;
    virtual int id() const OVERRIDE FINAL;
    virtual bool mergeWith(const QUndoCommand * command) OVERRIDE FINAL;
    void warpKeys();

private:
    boost::scoped_ptr<Curve::KeyFrameWarp> _warp;
    AnimItemDimViewKeyFramesMap _keys;
    std::vector<NodeAnimPtr> _nodes;
    std::vector<TableItemAnimPtr> _tableItems;
};


//////////////////////////////SET MULTIPLE KEYS INTERPOLATION COMMAND//////////////////////////////////////////////

/**
 * @class An undo command that can set the interpolation type of multiple keyframes of multiple curves
 **/
class SetKeysInterpolationCommand
    : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(SetKeysInterpolationCommand)

public:

    SetKeysInterpolationCommand(const AnimItemDimViewKeyFramesMap & keys,
                                KeyframeTypeEnum newInterpolation,
                                QUndoCommand *parent);

    virtual ~SetKeysInterpolationCommand() OVERRIDE
    {
    }

private:
    virtual void undo() OVERRIDE FINAL;
    virtual void redo() OVERRIDE FINAL;



private:
    KeysWithOldCurveMap _keys;
    KeyframeTypeEnum _newInterpolation;
};

/**
 * @class An undo command that can move derivatives of a control point of a curve
 **/
class MoveTangentCommand
    : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(MoveTangentCommand)

public:

    enum SelectedTangentEnum
    {
        eSelectedTangentLeft = 0,
        eSelectedTangentRight
    };


    MoveTangentCommand(SelectedTangentEnum deriv,
                       const AnimatingObjectIPtr& object,
                       DimIdx dimension,
                       ViewIdx view,
                       const KeyFrame& k,
                       double dx,
                       double dy,
                       QUndoCommand *parent = 0);

    MoveTangentCommand(SelectedTangentEnum deriv,
                       const AnimatingObjectIPtr& object,
                       DimIdx dimension,
                       ViewIdx view,
                       const KeyFrame& k,
                       double derivative,
                       QUndoCommand *parent = 0);

    virtual ~MoveTangentCommand() OVERRIDE
    {
    }

private:

    virtual void undo() OVERRIDE FINAL;
    virtual void redo() OVERRIDE FINAL;
    virtual int id() const OVERRIDE FINAL;
    virtual bool mergeWith(const QUndoCommand * command) OVERRIDE FINAL;

    void setNewDerivatives(bool undo);

private:

    AnimatingObjectIWPtr _object;
    DimIdx _dimension;
    ViewIdx _view;
    double _keyTime;
    SelectedTangentEnum _deriv;
    KeyframeTypeEnum _oldInterp, _newInterp;
    double _oldLeft, _oldRight, _newLeft, _newRight;
    bool _setBoth;
};



NATRON_NAMESPACE_EXIT;

#endif // NATRON_GUI_ANIMATION_MODULE_UNDO_REDO_H
