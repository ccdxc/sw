import { Component, Input, EventEmitter, Output } from '@angular/core';

@Component({
  selector: 'app-inlinebuttons',
  templateUrl: './inlinebuttons.component.html',
  styleUrls: ['./inlinebuttons.component.scss']
})
export class InlineButtonsComponent {
  @Input() saveTooltip: string;
  @Input() saveButtonClass: string;
  @Output() saveFunc = new EventEmitter();
  @Output() cancelFunc = new EventEmitter();

  save() {
    this.saveFunc.emit();
  }

  cancel() {
    this.cancelFunc.emit();
  }
}
