import { Component, Input, OnInit, ViewEncapsulation } from '@angular/core';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { Animations } from '@app/animations';
import { OnChanges } from '@angular/core/src/metadata/lifecycle_hooks';
import { Router } from '@angular/router';
import { StatArrowDirection, CardStates } from '../basecard/basecard.component';

export interface Stat {
  value: any;
  description: string;
  tooltip?: string;
  url?: string;
}

interface Data {
  x: Array<any>;
  y: Array<any>;
}



export interface HeroCardOptions {
  title: string;
  firstStat: Stat;
  secondStat: Stat;
  thirdStat: Stat;
  backgroundIcon: Icon;
  themeColor?: string;
  icon: Icon;
  lastUpdateTime?: string;
  data?: Data;
  arrowDirection?: StatArrowDirection;
  timeRange?: string;
  cardState?: CardStates;
}

@Component({
  selector: 'app-herocard',
  templateUrl: './herocard.component.html',
  styleUrls: ['./herocard.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class HerocardComponent implements OnInit, OnChanges {
  hasHover: boolean = false;
  cardStates = CardStates;

  @Input() title: string;
  @Input() firstStat: Stat;
  @Input() secondStat: Stat;
  @Input() thirdStat: Stat;
  @Input() themeColor: string;
  @Input() statColor: string = '#77a746';
  @Input() backgroundIcon: Icon;
  @Input() icon: Icon;
  @Input() lastUpdateTime;
  @Input() data: Data;
  @Input() arrowDirection: StatArrowDirection = StatArrowDirection.HIDDEN;
  @Input() timeRange: string;
  // When set to true, card contents will fade into view
  @Input() cardState: CardStates = CardStates.LOADING;


  showGraph: boolean = false;
  layout = {
    showlegend: false,
    paper_bgcolor: 'rgba(0,0,0,0)',
    plot_bgcolor: 'rgba(0,0,0,0)',
    autosize: true,
    margin: {
      t: 0,
      l: 0,
      r: 0,
      b: 0
    },
    width: 365,
    height: 40,
    yaxis: {
      autorange: false,
      range: [0, 100],
      showgrid: false,
      zeroline: false,
      showline: false,
      autotick: true,
      ticks: '',
      showticklabels: false
    },
    xaxis: {
      autorange: true,
      showgrid: false,
      zeroline: false,
      showline: false,
      autotick: true,
      ticks: '',
      showticklabels: false
    }
  };

  options = { displayModeBar: false, staticPlot: true };

  dataset = [];

  constructor(private router: Router) { }

  ngOnChanges(changes) {
    if (this.cardState === CardStates.READY) {
      this.setupDataset();
    }
  }

  ngOnInit() {
    if (this.cardState === CardStates.READY) {
      this.setupDataset();
    }
  }


  private setupDataset() {
    // If we only have one data point, we don't show the graph
    if (this.data.x.length <= 1) {
      this.showGraph = false;
      return;
    }
    this.dataset = [
      // passed in graph data
      {
        x: this.data.x,
        y: this.data.y,
        type: 'scatter',
        mode: 'lines',
        line: {
          shape: 'linear',
          color: this.themeColor,
          width: 1
        }
      },
      // second trace to add marker on last point
      {
        x: [this.data.x[this.data.x.length - 1]],
        y: [this.data.y[this.data.y.length - 1]],
        type: 'scatter',
        mode: 'marker',
        line: {
          color: this.themeColor,
        }
      }
    ];
    this.showGraph = true;
  }

  firstStatClick() {
    if (this.firstStat.value != null && this.firstStat.url != null) {
      this.router.navigateByUrl(this.firstStat.url);
    }
  }

  secondStatClick() {
    if (this.secondStat.value != null && this.secondStat.url != null) {
      this.router.navigateByUrl(this.secondStat.url);
    }
  }

  thirdStatClick() {
    if (this.thirdStat.value != null && this.thirdStat.url != null) {
      this.router.navigateByUrl(this.thirdStat.url);
    }
  }

}
