import { Component, Input, OnInit, ViewEncapsulation } from '@angular/core';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { Animations } from '@app/animations';
import { OnChanges } from '@angular/core/src/metadata/lifecycle_hooks';
import { Router } from '@angular/router';

export interface Stat {
  value: any;
  description: string;
  tooltip?: string;
  url?: string;
  onClick?: () => void;
  arrowDirection?: StatArrowDirection;
  statColor?: string;
}

export enum StatArrowDirection {
  UP = 'UP',
  DOWN = 'DOWN',
  HIDDEN = 'HIDDEN'
}

export enum CardStates {
  READY = 'READY',
  LOADING = 'LOADING',
  FAILED = 'FAILED'
}

export interface BaseCardOptions {
  title: string;
  backgroundIcon: Icon;
  themeColor?: string;
  icon: Icon;
  lastUpdateTime?: string;
  timeRange?: string;
  cardState?: CardStates;
}

@Component({
  selector: 'app-basecard',
  templateUrl: './basecard.component.html',
  styleUrls: ['./basecard.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class BasecardComponent implements OnInit, OnChanges {
  hasHover: boolean = false;
  cardStates = CardStates;

  @Input() title: string;
  @Input() themeColor: string;
  @Input() statColor: string = '#77a746';
  @Input() backgroundIcon: Icon;
  @Input() icon: Icon;
  @Input() lastUpdateTime;
  @Input() timeRange: string;
  // When set to true, card contents will fade into view
  @Input() cardState: CardStates = CardStates.LOADING;
  @Input() menuItems = [];


  showGraph: boolean = false;

  constructor(private router: Router) { }

  ngOnChanges(changes) {
  }

  ngOnInit() {
  }
}
