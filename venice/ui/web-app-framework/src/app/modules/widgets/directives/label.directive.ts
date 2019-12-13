import { Directive, Renderer2, ElementRef, OnInit, Input } from '@angular/core';

@Directive({
  selector: '[appFloatLabel]'
})
export class LabelDirective implements OnInit {
  constructor(private renderer: Renderer2, private el: ElementRef) {}
  @Input() appFloatLabel: string = '';

  ngOnInit() {
    if (!this.appFloatLabel) {
      return
    }
    const parent = this.el.nativeElement.parentNode;
    if (parent == null) {
      return;
    }
    this.renderer.addClass(parent, 'ui-float-label');

    const id = 'float-' + this.appFloatLabel + '-' + this.s4() + '-' + this.s4();
    this.renderer.setAttribute(this.el.nativeElement, 'id', id);
    const div = this.renderer.createElement('label');
    this.renderer.setAttribute(div, 'for', id);
    const text = this.renderer.createText(this.appFloatLabel);
    this.renderer.appendChild(div, text);

    this.renderer.appendChild(parent, div);
  }

  s4(): string {
    return Math.floor((1 + Math.random()) * 0x10000)
      .toString(16)
      .substring(1);
  }
}
