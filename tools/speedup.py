import os.path as osp
import pandas as pd
import matplotlib.pyplot as plt

RES_PATH = 'res'
MAX_THREAD = 64
DEFAULT_COL_NAMES = ["method", "n_threads", "n_fixes", "n_colors", "time", "n_conf"]


def plot(path: str) -> None:
    name = osp.basename(path).split('.')[0]
    
    df = pd.read_csv(path)
    df.rename(columns=dict(zip(df.columns, DEFAULT_COL_NAMES)), inplace=True)
    
    df['speedup'] = df.iloc[1]['time'].min() / df['time']
    seq = df.iloc[0]
    df = df.iloc[1:]
    
    ax = df.plot(x='n_threads', y='speedup', title=name, grid=True, style='*-',
                 label='Parallel Ver.')
    ax.axhline(y=seq['speedup'], color='g', linestyle='-.', zorder=0, 
                   label='Sequential Baseline')
    ax.plot([1, MAX_THREAD], [1, MAX_THREAD], 'r:', zorder=0, 
                label='Theoretical Upper Bound (y=x)')
    ax.plot([1, MAX_THREAD], [1, 1/MAX_THREAD], 'm:', zorder=0, 
            label='Theoretical Lower Bound (y=1/x)')
    
    ax.set_ylim([1/(MAX_THREAD), MAX_THREAD]); ax.set_xlim([1, MAX_THREAD])
    ax.set_yscale('log', base=2); ax.set_xscale('log', base=2)
    
    ax.set_ylabel('Speedup'); ax.legend(loc='lower left')
    plt.savefig(f'{RES_PATH}/{name}_time.png', dpi=300)
    plt.clf()

    ax = df.plot(x='n_threads', y='n_colors', title=name, grid=True, style='*-',
                 label='Parallel Ver.')
    ax.set_xscale('log', base=2); ax.set_ylim(0, 100)
    ax.axhline(y=seq['n_colors'], color='g', linestyle='-.', zorder=0, 
                   label='Sequential Baseline')
    ax.set_ylabel('No. Colors'); ax.legend(loc='lower left')
    plt.savefig(f'{RES_PATH}/{name}_colors.png', dpi=300)
    
    

if __name__ == "__main__":
    for f in ['log/nlpkkt80.log', 'log/nlpkkt120.log']:
        plot(f)