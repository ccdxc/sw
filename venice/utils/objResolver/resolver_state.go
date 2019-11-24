package objResolver

import apiintf "github.com/pensando/sw/api/interfaces"

type resolveState int

const (
	inprogress       resolveState = iota //Object op in progress
	unresolvedAdd                        //Object added but dep unsolved
	unresolvedUpdate                     //Object update but dep unsolved
	resolved                             //Object fully resolved
	unresolvedDelete                     //Object delete initiated but referrers are stil presebt
	markedForDelete                      //Object delete initiated but referrers are stil presebt
	deleted                              //Object delete initiated but referrers are stil presebt
)

func isAddUnResolved(state resolveState) bool {
	return state == unresolvedAdd
}

func isInProgress(state resolveState) bool {
	return state == inprogress
}

func isUpdateUnResolved(state resolveState) bool {
	return state == unresolvedUpdate
}

func isMarkedForDelete(state resolveState) bool {
	return state == markedForDelete
}

func isDeleted(state resolveState) bool {
	return state == deleted
}

func isResolved(state resolveState) bool {
	return state == resolved
}

func isDeleteUnResolved(state resolveState) bool {
	return state == unresolvedDelete
}

//ResolveCtx keeps context of resolved
type ResolveCtx struct {
	state       resolveState         //resolve state
	pendingObjs []apiintf.CtkitEvent // pending events when objet is in unresolved delete
}

//IsResolved is object resolved
func (ctx *ResolveCtx) IsResolved() bool {
	return isResolved(ctx.state)
}

//IsDelUnResolved is object delete unresolved
func (ctx *ResolveCtx) IsDelUnResolved() bool {
	return isDeleteUnResolved(ctx.state)
}

//IsAddUnResolved is object  add unresolved
func (ctx *ResolveCtx) IsAddUnResolved() bool {
	return isAddUnResolved(ctx.state)
}

//InProgress in progress
func (ctx *ResolveCtx) InProgress() bool {
	return isInProgress(ctx.state)
}

//IsUpdateUnResolved is update unresolved
func (ctx *ResolveCtx) IsUpdateUnResolved() bool {
	return isUpdateUnResolved(ctx.state)
}

//SetResolved  set resolved
func (ctx *ResolveCtx) SetResolved() {
	ctx.state = resolved
}

//SetInProgress set in progress so that
func (ctx *ResolveCtx) SetInProgress() {
	ctx.state = inprogress
}

//SetAddUnResolved set add unresolved
func (ctx *ResolveCtx) SetAddUnResolved() {
	ctx.state = unresolvedAdd
}

//IsMarkedForDelete is marked for delete
func (ctx *ResolveCtx) IsMarkedForDelete() bool {
	return isMarkedForDelete(ctx.state)
}

//IsDeleted is deleted
func (ctx *ResolveCtx) IsDeleted() bool {
	return isDeleted(ctx.state)
}

//IsInProgress is deleted
func (ctx *ResolveCtx) IsInProgress() bool {
	return isInProgress(ctx.state)
}

//IsOperationPending operation is pending
func (ctx *ResolveCtx) IsOperationPending() bool {
	return (ctx.InProgress() || ctx.IsDelUnResolved() || ctx.IsMarkedForDelete())
}

//SetDeleted  objected deleted
func (ctx *ResolveCtx) SetDeleted() {
	ctx.state = deleted
}

//SetMarkedForDelete  marked for delete
func (ctx *ResolveCtx) SetMarkedForDelete() {
	ctx.state = markedForDelete
}

//SetDeleteUnResolved  set delete un resolved
func (ctx *ResolveCtx) SetDeleteUnResolved() {
	ctx.state = unresolvedDelete
}

//SetUpdateUnResolved  set update un esolved
func (ctx *ResolveCtx) SetUpdateUnResolved() {
	ctx.state = unresolvedUpdate
}

//AddToPending  Add to pending
func (ctx *ResolveCtx) AddToPending(event apiintf.CtkitEvent) {
	ctx.pendingObjs = append(ctx.pendingObjs, event)
}

//PendingEvents Pendign events
func (ctx *ResolveCtx) PendingEvents() []apiintf.CtkitEvent {
	return ctx.pendingObjs
}

//ClearPendingEvents clear Pending events
func (ctx *ResolveCtx) ClearPendingEvents() {
	ctx.pendingObjs = []apiintf.CtkitEvent{}

}
