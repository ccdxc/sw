import { Directive, Input, ViewContainerRef, TemplateRef, EmbeddedViewRef, ɵstringify as stringify, OnDestroy } from '@angular/core';
import { UIConfigsService, Features } from '@app/services/uiconfigs.service';
import { ControllerService } from '@app/services/controller.service';
import { Utility } from '@app/common/Utility';
import { Subscription } from 'rxjs';
import { Eventtypes } from '@app/enum/eventtypes.enum';

export class FeatureGuardContext {
  public $implicit: FeatureGuardReq = null;
}

export interface FeatureGuardInput {
  // If missing any required features, the
  // component will not be displayed
  // Otherwise we check optionals, and if all of the listed features are
  // disabled we then disable
  // Else, it is enabled
  req?: Features[] | Features;
  opt?: Features[] | Features;
  // If checkDisabled is true, the directive is inverted.
  // If the feature is not enabled, then
  // it will render the template
  checkDisabled?: boolean;
}

export interface FeatureGuardReq {
  // If missing any required features, the
  // component will not be displayed
  // Otherwise we check optionals, and if all of the listed features are
  // disabled we then disable
  // Else, it is enabled
  req?: Features[];
  opt?: Features[];
  // If checkDisabled is true, the directive is inverted.
  // If the feature is not enabled, then
  // it will render the template
  checkDisabled?: boolean;
}

@Directive({
  selector: '[featureGuard]' // tslint:disable-line
})
export class FeatureGuardDirective implements OnDestroy {
  private _context: FeatureGuardContext = new FeatureGuardContext();
  private _thenTemplateRef: TemplateRef<FeatureGuardContext> | null = null;
  private _elseTemplateRef: TemplateRef<FeatureGuardContext> | null = null;
  private _thenViewRef: EmbeddedViewRef<FeatureGuardContext> | null = null;
  private _elseViewRef: EmbeddedViewRef<FeatureGuardContext> | null = null;

  subscriptions: Subscription[] = [];
  // the role the user must have
  @Input()
  set featureGuard(condition: FeatureGuardInput | Features) {
    if (condition == null || condition === '') {
      condition = {};
    }

    let featureReq: FeatureGuardReq;
    const _ = Utility.getLodash();
    if (typeof condition === 'string') {
      featureReq = {
        req: [condition]
      };
    } else {
      const input = condition as FeatureGuardInput;
      if (typeof input.req === 'string') {
        input.req = [input.req];
      }
      if (typeof input.opt === 'string') {
        input.opt = [input.opt];
      }
      featureReq = input as FeatureGuardReq;
    }
    this._context.$implicit = featureReq;
    if (featureReq.req != null) {
      featureReq.req.forEach( (p) => {
        if (Features[p] == null) {
          console.error('FeatureGuardDirective set featureGuard - req contained invalid UI feature', p);
        }
      });
    }
    if (featureReq.opt != null) {
      featureReq.opt.forEach( (p) => {
        if (Features[p] == null) {
          console.error('FeatureGuardDirective set featureGuard - opt contained invalid UI feature', p);
        }
      });
    }
    this._updateView();
  }

  /**
   * A template to show if the condition expression evaluates to true.
   */
  @Input()
  set featureGuardThen(templateRef: TemplateRef<FeatureGuardContext> | null) {
    assertTemplate('roleGuardThen', templateRef);
    this._thenTemplateRef = templateRef;
    this._thenViewRef = null;  // clear previous view if any.
    this._updateView();
  }

  /**
   * A template to show if the condition expression evaluates to false.
   */
  @Input()
  set featureGuardElse(templateRef: TemplateRef<FeatureGuardContext> | null) {
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
    const sub = this.controllerService.subscribe(Eventtypes.NEW_FEATURE_PERMISSIONS, (payload) => {
      // Check that this roleguard has received its input already
      // if not, we don't need to do an update
      if (this._context.$implicit != null) {
        this._updateView();
      }
    });
    this.subscriptions.push(sub);
  }

  _updateView() {
    const isEnabled = this.uiconfigService.featureGuardIsEnabled(this._context.$implicit.req, this._context.$implicit.opt);
    // Value may be null, so we double negate to get boolean values
    const checkEnabled = !!this._context.$implicit.checkDisabled;
    if (isEnabled === !checkEnabled) {
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
