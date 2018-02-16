import { Component, OnInit, OnDestroy,  ViewEncapsulation, ElementRef, Input, OnChanges, Output, EventEmitter } from '@angular/core';
import {Utility} from '../../../common/Utility';
@Component({
  selector: 'app-workloadwidget',
  templateUrl: './workloadwidget.component.html',
  styleUrls: ['./workloadwidget.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class WorkloadwidgetComponent implements OnInit, OnDestroy {


  @Input()  data: Array<any>;
  @Input()  layout: Array<any>;
  @Input()  options: Array<any>;
  @Input()  content: String;
  @Input()  id: String;

  @Output() widgetHeaderClick: EventEmitter<any> = new EventEmitter();
  constructor() {

   }

  ngOnInit() {
    if (!this.id) {

      this.id = 'workload-' + Utility.s4();
    }
  }

  ngOnDestroy() {

  }

  itemClick($event) {
    this.widgetHeaderClick.emit(this.id);

  }

}
