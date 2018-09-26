import { Component, OnInit, Output, EventEmitter, Input, ViewEncapsulation } from '@angular/core';

@Component({
  selector: 'app-authpolicybase',
  templateUrl: './authpolicybase.component.html',
  styleUrls: ['./authpolicybase.component.scss'],
  encapsulation: ViewEncapsulation.None,
  host: {
    '(mouseenter)': 'onMouseEnter()',
    '(mouseleave)': 'onMouseLeave()'
  }
})
export class AuthpolicybaseComponent implements OnInit {
  isHover: boolean = false;

  // current rank, zero indexed
  @Input() currentRank: number;

  // Total number of ranks
  @Input() numRanks: number;

  // Emits a request for a rank change, the parent is responsible for
  // actually changing the rank
  @Output() changeAuthRank: EventEmitter<number> = new EventEmitter();

  constructor() { }

  ngOnInit() {
  }

  onMouseEnter() {
    this.isHover = true;
  }

  onMouseLeave() {
    this.isHover = false;
  }

  canRaiseAuthRank(): boolean {
    if (this.currentRank == null || this.numRanks == null) {
      return false;
    }
    return this.currentRank > 0;
  }

  canLowerAuthRank(): boolean {
    if (this.currentRank == null || this.numRanks == null) {
      return false;
    }
    return this.currentRank < this.numRanks - 1;
  }

  raiseAuthRank() {
    if (this.canRaiseAuthRank()) {
      this.changeAuthRank.emit(this.currentRank - 1);
    }
  }

  lowerAuthRank() {
    if (this.canLowerAuthRank()) {
      this.changeAuthRank.emit(this.currentRank + 1);
    }
  }
}
