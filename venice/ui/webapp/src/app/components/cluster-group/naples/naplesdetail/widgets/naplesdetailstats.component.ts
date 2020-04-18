import { Component, OnInit, ViewEncapsulation, Input, OnChanges,
    SimpleChanges, ViewChild } from '@angular/core';
import { LinegraphComponent, LineGraphStat, GraphPadding } from '@app/components/shared/linegraph/linegraph.component';
import { CardStates } from '@app/components/shared/basecard/basecard.component';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';

@Component({
  selector: 'app-naplesdetail-stats',
  templateUrl: 'naplesdetailstats.component.html',
  styleUrls: ['naplesdetailstats.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})

export class NaplesdetailstatsComponent implements OnInit, OnChanges {
  @ViewChild('cpuLineGraph') cpuLineGraphComponent: LinegraphComponent;
  @ViewChild('memoryLineGraph') memoryLineGraphComponent: LinegraphComponent;
  @ViewChild('storageLineGraph') storageLineGraphComponent: LinegraphComponent;

  @Input() heroCards: any[] = [];
  @Input() lastUpdateTime: string = '';

  showGraph: boolean = false;
  linegraphStats: LineGraphStat[][] = [];
  lineGraphDataLength: number;

  graphPadding: GraphPadding = {
    top: 5,
    right: 10,
    bottom: 10,
    left: 10,
  };
  selectedIndex: number = 0;

  constructor() { }

  selectRow(event, rownum: number) {
    this.selectedIndex = rownum;
    this.updateGraph();
  }

  getTextClass() {
    if (this.selectedIndex === 1) {
      return 'memory';
    }
    if (this.selectedIndex === 2) {
      return 'storage';
    }
    return 'cpu';
  }

  ngOnInit() {
  }

  ngOnChanges(changes: SimpleChanges) {
    if (changes && changes.lastUpdateTime) {
      if (!this.showGraph) {
        this.checkGraphReady();
      }
      this.updateGraph();
    }
  }

  updateGraph() {
    if (this.showGraph) {
      const curCard = this.heroCards[this.selectedIndex];
      if (curCard && curCard.lineData && curCard.lineData.data) {
        curCard.lineData = this.pumpoutData(curCard.lineData);
        if (this.cpuLineGraphComponent && this.selectedIndex === 0) {
          this.cpuLineGraphComponent.setupCharts();
        }
        if (this.memoryLineGraphComponent && this.selectedIndex === 1) {
          this.memoryLineGraphComponent.setupCharts();
        }
        if (this.storageLineGraphComponent && this.selectedIndex === 2) {
          this.storageLineGraphComponent.setupCharts();
        }
      }
    }
  }

  // make cpu, memory, storage has same count of numbers as the initial account
  pumpoutData(lineData: LineGraphStat): LineGraphStat  {
    const newArray: any[] = [...lineData.data];
    if (lineData.data.length > this.lineGraphDataLength) {
      for (let i = 0; i < lineData.data.length - this.lineGraphDataLength; i++) {
        newArray.shift();
      }
    }
    lineData.data = newArray;
    return lineData;
  }

  checkGraphReady() {
    if (this.heroCards.length === 3) {
      // check whether every card is ready and lineData is not null
      const card = this.heroCards.find(item => {
        return item.cardState !== CardStates.READY ||
          !item.lineData || !item.lineData.data ||
          item.lineData.data.length === 0;
      });
      if (!card) {
        this.showGraph = true;
        this.lineGraphDataLength = this.heroCards[0].lineData.data.length;
        this.linegraphStats = [
          [this.heroCards[0].lineData],
          [this.heroCards[1].lineData],
          [this.heroCards[2].lineData]
        ];
      }
    }
  }

  /*********************************************************************
   * start section of caculate curve of cpu, memory, staoage usage     *
   *********************************************************************/
  polarToCartesian(centerX: number, centerY: number, radius: number, angleInDegrees: number) {
    const angleInRadians = (angleInDegrees - 90) * Math.PI / 180.0;

    return {
      x: centerX + (radius * Math.cos(angleInRadians)),
      y: centerY + (radius * Math.sin(angleInRadians))
    };
  }

  describeArc(x: number, y: number, radius: number, startAngle: number, endAngle: number): string {
    const start = this.polarToCartesian(x, y, radius, endAngle);
    const end = this.polarToCartesian(x, y, radius, startAngle);
    const largeArcFlag = endAngle - startAngle <= 180 ? '0' : '1';
    const d = [
        'M', start.x, start.y,
        'A', radius, radius, 0, largeArcFlag, 0, end.x, end.y
    ].join(' ');
    return d;
  }

  calculateDbyPercentage(percent: number, radius: number) {
    const newAngle = 310 * percent / 100;
    return this.describeArc(140, 140, radius, 25, 25 + newAngle);
  }

}

