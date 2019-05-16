import { ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { AlertlistComponent } from './alertlist.component';
import { AlertlistitemComponent } from './alertlistitem.component';
import { SharedModule } from '@app/components/shared//shared.module';

describe('AlertlistComponent', () => {
  let component: AlertlistComponent;
  let fixture: ComponentFixture<AlertlistComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [
        AlertlistComponent,
        AlertlistitemComponent
      ],
      imports: [
        // Other modules...
        SharedModule
      ],
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(AlertlistComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
