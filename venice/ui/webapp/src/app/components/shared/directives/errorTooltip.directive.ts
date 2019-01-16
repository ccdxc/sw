import { AriaDescriber, FocusMonitor } from '@angular/cdk/a11y';
import { Directionality } from '@angular/cdk/bidi';
import { Overlay } from '@angular/cdk/overlay';
import { Platform } from '@angular/cdk/platform';
import { ScrollDispatcher } from '@angular/cdk/scrolling';
import { Directive, ElementRef, Inject, NgZone, Optional, ViewContainerRef, OnInit } from '@angular/core';
import { NgControl } from '@angular/forms';
import { MatTooltip, MatTooltipDefaultOptions, MAT_TOOLTIP_DEFAULT_OPTIONS, MAT_TOOLTIP_SCROLL_STRATEGY } from '@angular/material';
import { HammerLoader, HAMMER_LOADER } from '@angular/platform-browser';

@Directive({
  selector: '[app-errorTooltip]',
  exportAs: 'errorTooltip',
  host: {
    '(longpress)': 'show()',
    '(keydown)': '_handleKeydown($event)',
    '(touchend)': '_handleTouchend()',
  },
})
export class ErrorTooltipDirective extends MatTooltip implements OnInit {
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

  ngOnInit() {
    this.tooltipClass = 'global-error-tooltip';
  }

  // Overriding from MatTooltip
  get message() {
    if (!this.control) {
      return;
    }
    const msgs = [];
    if (this.control.invalid && this.control.dirty) {
      for (const key in this.control.errors) {
        if (this.control.errors.hasOwnProperty(key)) {
          const error = this.control.errors[key];
          if (error.message != null) {
            msgs.push(error.message);
          }
        }
      }
    }
    return msgs.join('\n');
  }
}
