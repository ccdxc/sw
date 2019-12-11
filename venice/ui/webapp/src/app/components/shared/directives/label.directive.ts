import { Directive, Renderer2, ElementRef, OnInit, Input } from '@angular/core';
import { Utility } from '@app/common/Utility';

@Directive({
  selector: '[appFloatLabel]'
})
export class LabelDirective implements OnInit {
  constructor(private renderer: Renderer2, private el: ElementRef) {}
  @Input() floatLabel: string = '';

  ngOnInit() {
    const parent = this.el.nativeElement.parentNode;
    if (parent == null) {
      return;
    }
    this.renderer.addClass(parent, 'ui-float-label');

    const id = 'float-' + this.floatLabel + '-' + Utility.s4() + '-' + Utility.s4();
    this.renderer.setAttribute(this.el.nativeElement, 'id', id);
    const div = this.renderer.createElement('label');
    this.renderer.setAttribute(div, 'for', id);
    const text = this.renderer.createText(this.floatLabel);
    this.renderer.appendChild(div, text);

    this.renderer.appendChild(parent, div);
  }
}
