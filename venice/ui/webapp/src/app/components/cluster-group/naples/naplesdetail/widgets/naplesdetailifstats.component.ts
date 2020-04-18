import { Component, OnInit, ViewEncapsulation, Input, EventEmitter, Output } from '@angular/core';
import { InterfaceStats } from '../naplesdetail.component';

@Component({
  selector: 'app-naplesdetail-ifstats',
  templateUrl: 'naplesdetailifstats.component.html',
  styleUrls: ['naplesdetailifstats.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class NaplesdetailIfstatsComponent implements OnInit {
  @Input() interfaceStats: InterfaceStats[];
  @Input() mouseOverInterface: string = '';

  @Output() collapseExpandClickEmit: EventEmitter<string> = new EventEmitter();

  constructor() { }

  ngOnInit() {
  }

  getStats() {
    let ifs: InterfaceStats = null;
    if (this.interfaceStats && this.mouseOverInterface) {
      ifs = this.interfaceStats.find(item => item.ifname === this.mouseOverInterface);
    }
    return ifs;
  }

  expandInterfaceTable() {
    this.collapseExpandClickEmit.emit('expand');
  }

  displayStatsNumber (bytes: number): string {
    if (bytes === -1) {
      return '';
    }
    if (bytes === 0) {
      return '0 Bytes';
    }

    const k = 1024;
    const dm = 0;
    const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB'];

    const i = Math.floor(Math.log(bytes) / Math.log(k));

    return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + ' ' + sizes[i];
  }
}

