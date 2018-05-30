import { Component, Inject, ViewEncapsulation } from '@angular/core';
import { MAT_DIALOG_DATA, MatDialogRef } from '@angular/material';

@Component({
  selector: 'app-idlewarning',
  templateUrl: './idlewarning.component.html',
  encapsulation: ViewEncapsulation.None
})
export class IdleWarningComponent {

  constructor(
    public dialogRef: MatDialogRef<IdleWarningComponent>,
    @Inject(MAT_DIALOG_DATA) public data: any) { }

  onNoClick(): void {
    this.dialogRef.close();
  }

  updateCountdown(countdown): void {
    this.data.countdown = countdown;
  }
}
