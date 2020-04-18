import { Component, OnInit, ViewEncapsulation, Input, Output, EventEmitter } from '@angular/core';
import { InterfaceTopos } from '../naplesdetail.component';
@Component({
  selector: 'app-naplesdetail-iftopology',
  templateUrl: 'naplesdetailiftopology.component.html',
  styleUrls: ['naplesdetailiftopology.component.scss'],
  encapsulation: ViewEncapsulation.None
})

export class NaplesdetailIftopologyComponent implements OnInit {
  @Input() interfaceTopos: InterfaceTopos;
  @Input() interfaceStatsUpdateTime: string = '';
  @Input() mouseOverInterface: string = '';
  @Output() mouseOverInterfaceEmit: EventEmitter<string> = new EventEmitter();

  constructor() { }

  ngOnInit() {
  }

  setMouseOverInterface(itf: string) {
    this.mouseOverInterfaceEmit.emit(itf);
  }

}

