import { ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { AlertlistitemComponent } from './alertlistitem.component';
import { SharedModule } from '@app/components/shared//shared.module';

describe('AlertlistitemComponent', () => {
  let component: AlertlistitemComponent;
  let fixture: ComponentFixture<AlertlistitemComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [AlertlistitemComponent],
      imports: [
        // Other modules...
        SharedModule
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(AlertlistitemComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
