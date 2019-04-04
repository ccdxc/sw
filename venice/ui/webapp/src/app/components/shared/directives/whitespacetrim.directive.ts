import { Directive, HostListener, ElementRef } from '@angular/core';

const SPACEBAR = 32;
const TAB = 9;

/**
 * Prevents the user entering in white space in the
 * beginning of an input
 *
 * Automatically registers onto any inputs or textareas
 * Can opt out of it by adding the class penui-allow-spaces
 *
 * Escaping tslint since selector is not used as an attribute
 */
@Directive({
  /* tslint:disable */
  selector: `
    input:not([type=checkbox]):not([type=radio]):not([type=password]):not([readonly]):not(.ng-trim-ignore),
    textarea:not([readonly]):not(.penui-allow-spaces),
  `,
  /* tslint:enable */
})
export class WhitespaceTrimDirective {

  constructor(private el: ElementRef) {
    (el.nativeElement as HTMLInputElement).value = '';
  }

  @HostListener('keydown', ['$event'])
  onChange(event: KeyboardEvent) {
    if (event.keyCode === SPACEBAR) {
      if ((this.el.nativeElement as HTMLInputElement).value === '') {
        event.preventDefault();
      }
    }
  }
}
