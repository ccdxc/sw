import { AriaDescriber, FocusMonitor } from '@angular/cdk/a11y';
import { Directionality } from '@angular/cdk/bidi';
import { Overlay } from '@angular/cdk/overlay';
import { Platform } from '@angular/cdk/platform';
import { ScrollDispatcher } from '@angular/cdk/scrolling';
import { Directive, ElementRef, Inject, NgZone, Optional, ViewContainerRef, OnInit, Input } from '@angular/core';
import { NgControl } from '@angular/forms';
import { MatTooltip, MatTooltipDefaultOptions, MAT_TOOLTIP_DEFAULT_OPTIONS, MAT_TOOLTIP_SCROLL_STRATEGY } from '@angular/material';
import { HammerLoader, HAMMER_LOADER } from '@angular/platform-browser';

@Directive({
  selector: '[appErrorTooltip]',
  exportAs: 'errorTooltip',
  host: {
    '(longpress)': 'show()',
    '(keydown)': '_handleKeydown($event)',
    '(touchend)': '_handleTouchend()',
  },
})
export class ErrorTooltipDirective extends MatTooltip implements OnInit {
  tooltipDefaultClass: string = 'global-info-tooltip';
  tooltipErrorClass: string = 'global-error-tooltip';

  constructor(protected control: NgControl,
    // Following imports are for MatTooltip
    _overlay: Overlay,
    _elementRef: ElementRef<HTMLElement>,
    _scrollDispatcher: ScrollDispatcher,
    _viewContainerRef: ViewContainerRef,
    _ngZone: NgZone,
    platform: Platform,
    _ariaDescriber: AriaDescriber,
    _focusMonitor: FocusMonitor,
    @Inject(MAT_TOOLTIP_SCROLL_STRATEGY) scrollStrategy: any,
    @Optional() _dir: Directionality,
    @Optional() @Inject(MAT_TOOLTIP_DEFAULT_OPTIONS)
    _defaultOptions: MatTooltipDefaultOptions,
    @Optional() @Inject(HAMMER_LOADER) hammerLoader?: HammerLoader) {

    super(_overlay,
      _elementRef,
      _scrollDispatcher,
      _viewContainerRef,
      _ngZone,
      platform,
      _ariaDescriber,
      _focusMonitor,
      scrollStrategy,
      _dir,
      _defaultOptions,
      hammerLoader);
  }

  // Description to show on hover. Overrides any message picked up from swagger.
  @Input() appErrorTooltip: string;
  @Input() errorMessage: string = '';

  ngOnInit() {
  }

  // Overriding from MatTooltip
  get message() {
    if (!this.control) {
      this.tooltipClass = this.tooltipDefaultClass;
      return this.appErrorTooltip;
    }
    if (this.errorMessage.length > 0) {
      this.tooltipClass = this.tooltipErrorClass;
      return this.errorMessage;
    }
    if (this.control.touched && this.control.invalid && this.control.dirty) {
      const msgs = [];
      for (const key in this.control.errors) {
        if (this.control.errors.hasOwnProperty(key)) {
          const error = this.control.errors[key];
          if (error.message != null) {
            msgs.push(error.message);
          }
        }
      }
      this.tooltipClass = this.tooltipErrorClass;
      const ret = msgs.join('\n');
      return ret;
    }

    if (this.appErrorTooltip != null && this.appErrorTooltip !== '') {
      this.tooltipClass = this.tooltipDefaultClass;
      return this.appErrorTooltip;
    }
    // Description from swagger
    const desc = [];
    const customControl: any = this.control.control;
    if (customControl != null && customControl._venice_sdk != null && customControl._venice_sdk.description != null) {
      this.tooltipClass = this.tooltipDefaultClass;
      desc.push(customControl._venice_sdk.description);
    }
    if (customControl != null && customControl._venice_sdk != null && customControl._venice_sdk.hint != null) {
      this.tooltipClass = this.tooltipDefaultClass;
      desc.push( 'Ex. ' + customControl._venice_sdk.hint);
    }
    return desc.join('\n');
  }
}
