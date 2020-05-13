import { Component, OnInit, ViewEncapsulation, Input, Output, EventEmitter } from '@angular/core';
import { InterfaceStats } from '../naplesdetail.component';
import { Utility } from '@app/common/Utility';

@Component({
  selector: 'app-naplesdetail-iftable',
  templateUrl: 'naplesdetailiftable.component.html',
  styleUrls: ['naplesdetailiftable.component.scss'],
  encapsulation: ViewEncapsulation.None
})

export class NaplesdetailIftableComponent implements OnInit {
  @Input() interfaceStats: InterfaceStats[] = [];
  @Input() mouseOverInterface: string = '';
  @Output() collapseExpandClickEmit: EventEmitter<string> = new EventEmitter();

  constructor() { }

  ngOnInit() {
  }

  showStatsNumber (num: number): string {
    if (num === -1) {
      return '';
    }
    if (num === -1000) {
      return 'N/A';
    }
    return num === -1 ? '' : num.toString();
  }

  showStatsBytes (bytes: number): string {
    if (bytes === -1) {
      return '';
    }

    if (bytes === -1000) {
      return 'N/A';
    }

    return Utility.formatBytes(bytes, 2);
  }

  collapseInterfaceTable() {
    this.collapseExpandClickEmit.emit('collapse');
  }
}

