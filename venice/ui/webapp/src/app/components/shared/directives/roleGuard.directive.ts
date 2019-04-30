import { Directive, Input, ViewContainerRef, TemplateRef, EmbeddedViewRef, ɵstringify as stringify, OnDestroy } from '@angular/core';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { ControllerService } from '@app/services/controller.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Utility } from '@app/common/Utility';
import { Subscription } from 'rxjs';

export class RoleGuardContext {
  public $implicit: RoleGuardReq = null;
}

export interface RoleGuardInput {
  // If the user is missing any required permissions, the
  // component will not be displayed
  // Otherwise we check optionals, and if all of the listed objects are
  // disabled we then disable the route
  // Else, it is enabled
  req?: UIRolePermissions[] | UIRolePermissions;
  opt?: UIRolePermissions[] | UIRolePermissions;
  // If unauthorized is true, the directive is inverted.
  // If the user does NOT have the correct permissions, then
  // it will render the template
  checkUnauthorized?: boolean;
}

export interface RoleGuardReq {
  // If the user is missing any required permissions, the
  // component will not be displayed
  // Otherwise we check optionals, and if all of the listed objects are
  // disabled we then disable the route
  // Else, it is enabled
  req?: UIRolePermissions[];
  opt?: UIRolePermissions[];
  // If unauthorized is true, the directive is inverted.
  // If the user does NOT have the correct permissions, then
  // it will render the template
  checkUnauthorized?: boolean;
}

@Directive({
  selector: '[roleGuard]' // tslint:disable-line
})
export class RoleGuardDirective implements OnDestroy {
  private _context: RoleGuardContext = new RoleGuardContext();
  private _thenTemplateRef: TemplateRef<RoleGuardContext> | null = null;
  private _elseTemplateRef: TemplateRef<RoleGuardContext> | null = null;
  private _thenViewRef: EmbeddedViewRef<RoleGuardContext> | null = null;
  private _elseViewRef: EmbeddedViewRef<RoleGuardContext> | null = null;

  subscriptions: Subscription[] = [];
  // the role the user must have
  @Input()
  set roleGuard(condition: RoleGuardInput | UIRolePermissions) {
    if (condition == null || condition === '') {
      condition = {};
    }

    let roleReq: RoleGuardReq;
    const _ = Utility.getLodash();
    if (typeof condition === 'string') {
      roleReq = {
        req: [condition]
      };
    } else {
      const input = condition as RoleGuardInput;
      if (typeof input.req === 'string') {
        input.req = [input.req];
      }
      if (typeof input.opt === 'string') {
        input.opt = [input.opt];
      }
      roleReq = input as RoleGuardReq;
    }
    this._context.$implicit = roleReq;
    if (roleReq.req != null) {
      roleReq.req = roleReq.req.map( (p) => {
        return _.toLower(p) as UIRolePermissions;
      });
      roleReq.req.forEach( (p) => {
        if (UIRolePermissions[p] == null) {
          console.error('RoleGuardDirective set RoleGuard - req contained invalid UI permission', p);
        }
      });
    }
    if (roleReq.opt != null) {
      roleReq.opt = roleReq.opt.map( (p) => {
        return _.toLower(p) as UIRolePermissions;
      });
      roleReq.opt.forEach( (p) => {
        if (UIRolePermissions[p] == null) {
          console.error('RoleGuardDirective set RoleGuard - opt contained invalid UI permission', p);
        }
      });
    }
    this._updateView();
  }

  /**
   * A template to show if the condition expression evaluates to true.
   */
  @Input()
  set roleGuardThen(templateRef: TemplateRef<RoleGuardContext> | null) {
    assertTemplate('roleGuardThen', templateRef);
    this._thenTemplateRef = templateRef;
    this._thenViewRef = null;  // clear previous view if any.
    this._updateView();
  }

  /**
   * A template to show if the condition expression evaluates to false.
   */
  @Input()
  set roleGuardElse(templateRef: TemplateRef<RoleGuardContext> | null) {
    assertTemplate('roleGuardElse', templateRef);
    this._elseTemplateRef = templateRef;
    this._elseViewRef = null;  // clear previous view if any.
    this._updateView();
  }

  /**
   * @param {ViewContainerRef} viewContainerRef
   * 	-- the location where we need to render the templateRef
   * @param {TemplateRef<any>} templateRef
   *   -- the templateRef to be potentially rendered
   */
  constructor(
    private viewContainerRef: ViewContainerRef,
    private templateRef: TemplateRef<any>,
    private uiconfigService: UIConfigsService,
    private controllerService: ControllerService
  ) {
    this._thenTemplateRef = templateRef;
    const sub = this.controllerService.subscribe(Eventtypes.NEW_USER_PERMISSIONS, (payload) => {
      // Check that this roleguard has received its input already
      // if not, we don't need to do an update
      if (this._context.$implicit != null) {
        this._updateView();
      }
    });
    this.subscriptions.push(sub);
  }

  _updateView() {
    const isAuthorized = this.uiconfigService.roleGuardIsAuthorized(this._context.$implicit.req, this._context.$implicit.opt);
    // Value may be null, so we double negate to get boolean values
    const checkUnauthorized = !!this._context.$implicit.checkUnauthorized;
    if (isAuthorized === !checkUnauthorized) {
      if (!this._thenViewRef) {
        this.viewContainerRef.clear();
        this._elseViewRef = null;
        if (this._thenTemplateRef) {
          this._thenViewRef =
            this.viewContainerRef.createEmbeddedView(this._thenTemplateRef, this._context);
        }
      }
    } else {
      if (!this._elseViewRef) {
        this.viewContainerRef.clear();
        this._thenViewRef = null;
        if (this._elseTemplateRef) {
          this._elseViewRef =
            this.viewContainerRef.createEmbeddedView(this._elseTemplateRef, this._context);
        }
      }
    }
  }

  ngOnDestroy() {
    this.subscriptions.forEach( (sub) => {
      sub.unsubscribe();
    });
  }
}

function assertTemplate(property: string, templateRef: TemplateRef<any> | null): void {
  const isTemplateRefOrNull = !!(!templateRef || templateRef.createEmbeddedView);
  if (!isTemplateRefOrNull) {
    throw new Error(`${property} must be a TemplateRef, but received '${stringify(templateRef)}'.`);
  }
}
