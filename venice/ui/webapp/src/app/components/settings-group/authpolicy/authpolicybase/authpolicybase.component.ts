import { Component, OnInit, Output, EventEmitter, Input, ViewEncapsulation } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { AbstractControl } from '@angular/forms';
import { Utility } from '@app/common/Utility';

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

  constructor(protected _controllerService: ControllerService) { }

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

 /*  getControlTooltip(control: AbstractControl, field: string, defaultTooltip: string): string {
    const tooltopString = this.getControlValidationText(control, field, defaultTooltip);
    return Utility.getControlTooltip(control, field, defaultTooltip);
  } */

  /**
   * Get tooltip for control.
   * Extended class should override this API to get tooltip from class.propInfo
   * Say, the control is AuthRadius.server.url (AuthRadiusServer), it should get AuthRadiusServer.propInfo.url.description
   * @param control
   * @param field
   * @param defaultTooltip
   *
   *
   */
  getControlValidationText(control: AbstractControl, field: string, defaultTooltip: string): string {
    return defaultTooltip;
  }
}
