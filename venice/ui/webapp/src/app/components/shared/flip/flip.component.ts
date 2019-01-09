import { Component, OnInit, ViewEncapsulation, Input } from '@angular/core';

export enum FlipState {
  front = 'inactive',
  back = 'active'
}

@Component({
  selector: 'app-flip',
  templateUrl: './flip.component.html',
  styleUrls: ['./flip.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class FlipComponent implements OnInit {
  @Input() flipState: FlipState = FlipState.front;

  public static toggleState(currState): FlipState {
    if (currState === FlipState.front) {
      return FlipState.back;
    }
    return FlipState.front;
  }

  constructor() { }

  ngOnInit() {
  }

}
